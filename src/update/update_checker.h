#ifndef ANONYMUS_BROWSER_UPDATE_CHECKER_H_
#define ANONYMUS_BROWSER_UPDATE_CHECKER_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/timer/repeating_timer.h"
#include "url/gurl.h"

namespace anonymus {

// ============================================================
// Семантическое версионирование (SemVer)
// ============================================================
struct SemVer {
  int major = 0;
  int minor = 0;
  int patch = 0;
  std::string prerelease;  // "alpha", "beta", "rc1"
  std::string build;       // "build.123"

  SemVer() = default;
  SemVer(int maj, int min, int pat)
      : major(maj), minor(min), patch(pat) {}

  // Парсинг из строки "2.1.4" или "v2.1.4-beta.1"
  static SemVer Parse(const std::string& version_str);

  // Сравнение версий
  // Возвращает: -1 (this < other), 0 (равны), 1 (this > other)
  int Compare(const SemVer& other) const;

  // Операторы сравнения
  bool operator<(const SemVer& other) const { return Compare(other) < 0; }
  bool operator>(const SemVer& other) const { return Compare(other) > 0; }
  bool operator==(const SemVer& other) const { return Compare(other) == 0; }
  bool operator<=(const SemVer& other) const { return Compare(other) <= 0; }
  bool operator>=(const SemVer& other) const { return Compare(other) >= 0; }

  // Вывод в строку
  std::string ToString() const;
};

// ============================================================
// Информация о релизе
// ============================================================
struct ReleaseInfo {
  SemVer version;
  std::string name;           // "Anonymus Browser v2.1.4"
  std::string body;           // Release notes
  std::string published_at;   // ISO 8601
  bool prerelease = false;

  struct Asset {
    std::string name;         // "AnonymusSetup_x64.exe"
    std::string download_url;
    int64_t size = 0;
    std::string sha256;       // Хеш файла
  };

  std::vector<Asset> assets;
};

// ============================================================
// Результат проверки обновлений
// ============================================================
enum class UpdateStatus {
  kUpToDate,           // Уже последняя версия
  kUpdateAvailable,    // Доступно обновление
  kUpdateRequired,     // Критическое обновление (безопасность)
  kError,              // Ошибка проверки
  kRateLimited,        // Превышен лимит запросов
  kNetworkError        // Проблемы с сетью
};

struct UpdateResult {
  UpdateStatus status;
  ReleaseInfo release;       // Информация о новом релизе
  SemVer current_version;    // Текущая версия
  std::string error_message;
  int rate_limit_remaining = 0;
  int64_t rate_limit_reset = 0;  // Unix timestamp
};

// ============================================================
// Callback типы
// ============================================================
using UpdateCheckCallback = base::OnceCallback<void(const UpdateResult&)>;
using DownloadProgressCallback = base::RepeatingCallback<void(int64_t current, int64_t total)>;
using DownloadCompleteCallback = base::OnceCallback<void(bool success, const base::FilePath& path)>;

// ============================================================
// Проверщик обновлений
// ============================================================
class UpdateChecker {
 public:
  // GitHub API configuration
  static constexpr char kGitHubApiHost[] = "api.github.com";
  static constexpr char kReleasesEndpoint[] =
      "/repos/Anonymus-Browser/Anonymus/releases/latest";
  static constexpr char kApiVersion[] = "2022-11-28";

  UpdateChecker();
  ~UpdateChecker();

  UpdateChecker(const UpdateChecker&) = delete;
  UpdateChecker& operator=(const UpdateChecker&) = delete;

  // Singleton
  static UpdateChecker* GetInstance();

  // Инициализация (вызывается при старте браузера)
  void Initialize();

  // Ручная проверка обновлений
  void CheckForUpdates(UpdateCheckCallback callback);

  // Автоматическая проверка (таймер)
  void StartAutoCheck(base::TimeDelta interval = base::Hours(4));
  void StopAutoCheck();

  // Скачивание обновления
  void DownloadUpdate(const ReleaseInfo& release,
                      DownloadProgressCallback progress_callback,
                      DownloadCompleteCallback complete_callback);

  // Установка обновления
  bool ApplyUpdate(const base::FilePath& installer_path);

  // Отказ от обновления (с записью в лог)
  void DeclineUpdate(const SemVer& version);

  // Проверка: было ли отклонено обновление
  bool IsUpdateDeclined(const SemVer& version) const;

  // Текущая версия
  SemVer GetCurrentVersion() const;

  // Получение токена (read-only)
  std::string GetGitHubToken() const;

 private:
  // HTTP запрос к GitHub API
  void FetchLatestRelease(UpdateCheckCallback callback);

  // Парсинг JSON ответа
  ReleaseInfo ParseReleaseResponse(const std::string& json,
                                   std::string* error);

  // Проверка SHA-256 хеша файла
  bool VerifyChecksum(const base::FilePath& file_path,
                      const std::string& expected_sha256);

  // Запись в реестр
  void SaveDeclinedVersion(const SemVer& version);
  bool LoadDeclinedVersions();

  // Обработка rate limit
  void HandleRateLimit(int remaining, int64_t reset_time);

  // Получение текущей версии из реестра
  SemVer LoadCurrentVersion();
  void SaveCurrentVersion(const SemVer& version);

  // Таймер
  base::RepeatingTimer auto_check_timer_;
  bool initialized_ = false;

  // Текущая версия (кэш)
  SemVer current_version_;

  // Отклонённые версии
  std::vector<SemVer> declined_versions_;

  // Rate limit state
  int rate_limit_remaining_ = 60;
  int64_t rate_limit_reset_ = 0;

  base::WeakPtrFactory<UpdateChecker> weak_ptr_factory_{this};
};

// ============================================================
// Утилита: SHA-256 хеширование
// ============================================================
class Sha256Util {
 public:
  // Вычисление SHA-256 хеша файла
  static std::string HashFile(const base::FilePath& file_path);

  // Вычисление SHA-256 хеша строки
  static std::string HashString(const std::string& data);

  // Сравнение хешей (constant-time)
  static bool CompareHashes(const std::string& a, const std::string& b);
};

// ============================================================
// Утилита: Безопасное скачивание
// ============================================================
class SecureDownloader {
 public:
  // Скачивание с проверкой SSL пиннинга
  static bool Download(const GURL& url,
                       const base::FilePath& dest_path,
                       DownloadProgressCallback progress_callback);

  // SSL пиннинг для GitHub
  static bool VerifySSLCertificate(const std::string& cert_chain);

 private:
  // GitHub SSL публичный ключ (SHA-256)
  static const char* kGitHubCertFingerprint[];
};

}  // namespace anonymus

#endif  // ANONYMUS_BROWSER_UPDATE_CHECKER_H_
