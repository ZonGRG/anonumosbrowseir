#include "anonymus/update/update_checker.h"

#include <algorithm>
#include <sstream>

#include "base/base64.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/rand_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include "crypto/sha2.h"
#include "net/http/http_status_code.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "net/url_request/url_request_context.h"

// WinHTTP для скачивания
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

namespace anonymus {

// ============================================================
// SemVer
// ============================================================

SemVer SemVer::Parse(const std::string& version_str) {
  SemVer ver;
  std::string s = version_str;

  // Удаление префикса 'v'
  if (!s.empty() && (s[0] == 'v' || s[0] == 'V')) {
    s = s.substr(1);
  }

  // Разделение на MAJOR.MINOR.PATCH
  size_t pos = 0;
  std::string token;

  // Major
  pos = s.find('.');
  if (pos != std::string::npos) {
    token = s.substr(0, pos);
    ver.major = std::stoi(token);
    s = s.substr(pos + 1);
  }

  // Minor
  pos = s.find('.');
  if (pos != std::string::npos) {
    token = s.substr(0, pos);
    ver.minor = std::stoi(token);
    s = s.substr(pos + 1);
  } else {
    ver.minor = std::stoi(s);
    return ver;
  }

  // Patch (+prerelease)
  pos = s.find('-');
  if (pos != std::string::npos) {
    ver.patch = std::stoi(s.substr(0, pos));
    ver.prerelease = s.substr(pos + 1);

    // Разделение prerelease и build
    size_t build_pos = ver.prerelease.find('+');
    if (build_pos != std::string::npos) {
      ver.build = ver.prerelease.substr(build_pos + 1);
      ver.prerelease = ver.prerelease.substr(0, build_pos);
    }
  } else {
    ver.patch = std::stoi(s);
  }

  return ver;
}

int SemVer::Compare(const SemVer& other) const {
  // Сравнение MAJOR.MINOR.PATCH
  if (major != other.major) return major < other.major ? -1 : 1;
  if (minor != other.minor) return minor < other.minor ? -1 : 1;
  if (patch != other.patch) return patch < other.patch ? -1 : 1;

  // Прerelease: "alpha" < "beta" < "rc" < "" (пусто = стабильный)
  if (prerelease.empty() && other.prerelease.empty()) return 0;
  if (prerelease.empty()) return 1;   // Стабильный > prerelease
  if (other.prerelease.empty()) return -1;

  // Лексикографическое сравнение prerelease
  if (prerelease < other.prerelease) return -1;
  if (prerelease > other.prerelease) return 1;

  return 0;
}

std::string SemVer::ToString() const {
  std::string result = std::to_string(major) + "." +
                       std::to_string(minor) + "." +
                       std::to_string(patch);
  if (!prerelease.empty()) {
    result += "-" + prerelease;
  }
  return result;
}

// ============================================================
// UpdateChecker
// ============================================================

// static
const char UpdateChecker::kGitHubApiHost[] = "api.github.com";
const char UpdateChecker::kReleasesEndpoint[] =
    "/repos/Anonymus-Browser/Anonymus/releases/latest";
const char UpdateChecker::kApiVersion[] = "2022-11-28";

UpdateChecker::UpdateChecker() = default;

UpdateChecker::~UpdateChecker() = default;

// static
UpdateChecker* UpdateChecker::GetInstance() {
  static base::NoDestructor<UpdateChecker> instance;
  return instance.get();
}

void UpdateChecker::Initialize() {
  if (initialized_) return;

  current_version_ = LoadCurrentVersion();
  LoadDeclinedVersions();

  LOG(INFO) << "[Update] Initialized, current version: "
            << current_version_.ToString();

  initialized_ = true;
}

void UpdateChecker::CheckForUpdates(UpdateCheckCallback callback) {
  if (!initialized_) {
    Initialize();
  }

  LOG(INFO) << "[Update] Checking for updates...";

  // Проверка rate limit
  base::Time now = base::Time::Now();
  int64_t now_seconds = now.ToTimeT();
  if (rate_limit_remaining_ <= 0 && now_seconds < rate_limit_reset_) {
    UpdateResult result;
    result.status = UpdateStatus::kRateLimited;
    result.rate_limit_remaining_ = 0;
    result.rate_limit_reset_ = rate_limit_reset_;
    std::move(callback).Run(result);
    return;
  }

  FetchLatestRelease(std::move(callback));
}

void UpdateChecker::StartAutoCheck(base::TimeDelta interval) {
  auto_check_timer_.Start(
      FROM_HERE, interval,
      base::BindRepeating(&UpdateChecker::CheckForUpdates,
                          weak_ptr_factory_.GetWeakPtr(),
                          base::BindOnce([](const UpdateResult& result) {
                            if (result.status == UpdateStatus::kUpdateAvailable ||
                                result.status == UpdateStatus::kUpdateRequired) {
                              LOG(INFO) << "[Update] Auto-check: update available";
                            }
                          })));
}

void UpdateChecker::StopAutoCheck() {
  auto_check_timer_.Stop();
}

void UpdateChecker::FetchLatestRelease(UpdateCheckCallback callback) {
  // Формирование запроса
  GURL url(base::StringPrintf("https://%s%s", kGitHubApiHost, kReleasesEndpoint));

  // Запуск HTTP запроса в отдельном потоке
  auto task_runner = base::SequencedTaskRunner::GetCurrentDefault();

  task_runner->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](GURL url, UpdateCheckCallback cb) {
            // WinHTTP запрос
            HINTERNET hSession = WinHttpOpen(
                L"AnonymusBrowser/2.1.3 (Windows; x64)",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS,
                0);

            if (!hSession) {
              UpdateResult result;
              result.status = UpdateStatus::kError;
              result.error_message = "WinHttpOpen failed";
              std::move(cb).Run(result);
              return;
            }

            // Установка таймаутов
            WinHttpSetTimeouts(hSession, 5000, 5000, 15000, 30000);

            // Подключение
            HINTERNET hConnect = WinHttpConnect(
                hSession,
                L"api.github.com",
                INTERNET_DEFAULT_HTTPS_PORT,
                0);

            if (!hConnect) {
              WinHttpCloseHandle(hSession);
              UpdateResult result;
              result.status = UpdateStatus::kNetworkError;
              std::move(cb).Run(result);
              return;
            }

            // Создание запроса
            HINTERNET hRequest = WinHttpOpenRequest(
                hConnect,
                L"GET",
                L"/repos/Anonymus-Browser/Anonymus/releases/latest",
                NULL, WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                WINHTTP_FLAG_SECURE);

            if (!hRequest) {
              WinHttpCloseHandle(hConnect);
              WinHttpCloseHandle(hSession);
              UpdateResult result;
              result.status = UpdateStatus::kError;
              std::move(cb).Run(result);
              return;
            }

            // Добавление заголовков
            std::wstring headers =
                L"Accept: application/vnd.github+json\r\n"
                L"X-GitHub-Api-Version: 2022-11-28\r\n";

            // Отправка запроса
            BOOL result = WinHttpSendRequest(
                hRequest,
                headers.c_str(),
                (DWORD)headers.length(),
                WINHTTP_NO_REQUEST_DATA,
                0,
                0,
                0);

            if (!result) {
              WinHttpCloseHandle(hRequest);
              WinHttpCloseHandle(hConnect);
              WinHttpCloseHandle(hSession);
              UpdateResult ur;
              ur.status = UpdateStatus::kError;
              ur.error_message = "WinHttpSendRequest failed";
              std::move(cb).Run(ur);
              return;
            }

            // Получение ответа
            result = WinHttpReceiveResponse(hRequest, NULL);

            DWORD status_code = 0;
            DWORD size = sizeof(status_code);
            WinHttpQueryHeaders(
                hRequest,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,
                &status_code, &size, WINHTTP_NO_HEADER_INDEX);

            // Чтение тела ответа
            std::string response_body;
            DWORD bytes_available = 0;

            while (WinHttpQueryDataAvailable(hRequest, &bytes_available) &&
                   bytes_available > 0) {
              std::vector<char> buffer(bytes_available);
              DWORD bytes_read = 0;
              WinHttpReadData(hRequest, buffer.data(), bytes_available, &bytes_read);
              response_body.append(buffer.data(), bytes_read);
            }

            // Закрытие хендлов
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            // Обработка ответа
            if (status_code == 200) {
              // Парсинг JSON
              ReleaseInfo release;
              std::string error;

              // Простой парсинг (в реальности - JSON library)
              // Здесь заглушка для демонстрации
              release.version = SemVer::Parse("2.1.4");
              release.name = "Anonymus Browser v2.1.4";

              UpdateResult ur;
              ur.status = UpdateStatus::kUpdateAvailable;
              ur.release = release;
              std::move(cb).Run(ur);

            } else if (status_code == 304) {
              UpdateResult ur;
              ur.status = UpdateStatus::kUpToDate;
              std::move(cb).Run(ur);

            } else if (status_code == 403) {
              UpdateResult ur;
              ur.status = UpdateStatus::kRateLimited;
              ur.rate_limit_remaining_ = 0;
              std::move(cb).Run(ur);

            } else {
              UpdateResult ur;
              ur.status = UpdateStatus::kError;
              ur.error_message = "HTTP " + std::to_string(status_code);
              std::move(cb).Run(ur);
            }
          },
          url, std::move(callback)));
}

void UpdateChecker::DownloadUpdate(
    const ReleaseInfo& release,
    DownloadProgressCallback progress_callback,
    DownloadCompleteCallback complete_callback) {
  LOG(INFO) << "[Update] Downloading version: " << release.version.ToString();

  // Поиск x64 exe в assets
  std::string download_url;
  for (const auto& asset : release.assets) {
    if (asset.name.find("x64") != std::string::npos &&
        asset.name.ends_with(".exe")) {
      download_url = asset.download_url;
      break;
    }
  }

  if (download_url.empty()) {
    std::move(complete_callback).Run(false, base::FilePath());
    return;
  }

  // Скачивание в %TEMP%
  base::FilePath temp_dir;
  base::GetTempDir(&temp_dir);
  base::FilePath dest = temp_dir.Append(L"Anonymus_Update")
                                   .Append(L"AnonymusSetup_x64.exe");

  // Запуск скачивания в отдельном потоке
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](std::string url, base::FilePath dest,
             DownloadProgressCallback progress_cb,
             DownloadCompleteCallback complete_cb) {
            // WinHTTP скачивание с прогрессом
            // ... (реализация аналогична FetchLatestRelease)

            // Заглушка: создание пустого файла
            base::File file(dest, base::File::FLAG_CREATE | base::File::FLAG_WRITE);
            if (file.IsValid()) {
              file.Close();
              std::move(complete_cb).Run(true, dest);
            } else {
              std::move(complete_cb).Run(false, base::FilePath());
            }
          },
          download_url, dest,
          std::move(progress_callback),
          std::move(complete_callback)));
}

bool UpdateChecker::VerifyChecksum(const base::FilePath& file_path,
                                    const std::string& expected_sha256) {
  std::string actual_hash = Sha256Util::HashFile(file_path);
  return Sha256Util::CompareHashes(actual_hash, expected_sha256);
}

bool UpdateChecker::ApplyUpdate(const base::FilePath& installer_path) {
  LOG(INFO) << "[Update] Applying update from: " << installer_path;

  // Запуск установщика с флагами
  std::wstring cmd = installer_path.value() +
                     L" /UPDATE=1 /VERYSILENT /SUPPRESSMSGBOXES";

  STARTUPINFOW si = {sizeof(si)};
  PROCESS_INFORMATION pi = {};

  BOOL result = CreateProcessW(
      NULL,
      const_cast<wchar_t*>(cmd.c_str()),
      NULL, NULL, FALSE,
      CREATE_NO_WINDOW,
      NULL, NULL,
      &si, &pi);

  if (result) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Завершение текущего процесса
    base::Process::TerminateCurrentProcess(0);
    return true;
  }

  return false;
}

void UpdateChecker::DeclineUpdate(const SemVer& version) {
  LOG(WARNING) << "[Update] User declined update to " << version.ToString();

  declined_versions_.push_back(version);
  SaveDeclinedVersion(version);
}

bool UpdateChecker::IsUpdateDeclined(const SemVer& version) const {
  for (const auto& v : declined_versions_) {
    if (v == version) return true;
  }
  return false;
}

SemVer UpdateChecker::GetCurrentVersion() const {
  return current_version_;
}

std::string UpdateChecker::GetGitHubToken() const {
  // Публичный токен (read-only) для обхода rate limit
  // В продакшене: хранить в зашифрованном виде
  return "ghp_public_readonly_token";
}

void UpdateChecker::HandleRateLimit(int remaining, int64_t reset_time) {
  rate_limit_remaining_ = remaining;
  rate_limit_reset_ = reset_time;

  if (remaining <= 0) {
    LOG(WARNING) << "[Update] Rate limit reached, resets at: " << reset_time;
  }
}

SemVer UpdateChecker::LoadCurrentVersion() {
  // Загрузка из реестра
  // HKEY_CURRENT_USER\Software\Anonymus\Version
  return SemVer(2, 1, 3);  // Заглушка
}

void UpdateChecker::SaveCurrentVersion(const SemVer& version) {
  // Сохранение в реестр
  LOG(INFO) << "[Update] Version saved: " << version.ToString();
}

// ============================================================
// Sha256Util
// ============================================================

// static
std::string Sha256Util::HashFile(const base::FilePath& file_path) {
  base::File file(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid()) return "";

  crypto::SHA256HashValue hash;
  crypto::SHA256Init(&hash);

  char buffer[65536];
  int64_t bytes_read;
  while ((bytes_read = file.ReadAtCurrentPos(buffer, sizeof(buffer))) > 0) {
    crypto::SHA256Update(&hash, buffer, bytes_read);
  }

  return base::HexEncode(hash.data(), sizeof(hash.data));
}

// static
std::string Sha256Util::HashString(const std::string& data) {
  crypto::SHA256HashValue hash;
  crypto::SHA256Init(&hash);
  crypto::SHA256Update(&hash, data.data(), data.size());
  return base::HexEncode(hash.data(), sizeof(hash.data));
}

// static
bool Sha256Util::CompareHashes(const std::string& a, const std::string& b) {
  if (a.size() != b.size()) return false;

  // Constant-time comparison (защита от timing attacks)
  volatile uint8_t result = 0;
  for (size_t i = 0; i < a.size(); i++) {
    result |= static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]);
  }
  return result == 0;
}

// ============================================================
// SecureDownloader
// ============================================================

// static
const char* SecureDownloader::kGitHubCertFingerprint[] = {
    // SHA-256 fingerprints of GitHub's SSL certificates
    // Обновляется при смене сертификатов GitHub
    "sha256/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX=",
    nullptr
};

// static
bool SecureDownloader::VerifySSLCertificate(const std::string& cert_chain) {
  // Проверка SSL пиннинга для GitHub
  // В реальности: проверка сертификата через OpenSSL
  return true;  // Заглушка
}

}  // namespace anonymus
