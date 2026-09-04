# Anonymus Browser — Инструкция по сборке

## 1. Требования

### Системные требования
- **ОС:** Windows 10/11 (x64)
- **RAM:** минимум 16 GB (рекомендуется 32 GB)
- **Диск:** минимум 100 GB свободного места (SSD рекомендуется)
- **Процессор:** 4+ ядер

### Программное обеспечение

| Программа | Версия | Ссылка |
|-----------|--------|--------|
| Visual Studio | 2022+ | https://visualstudio.microsoft.com/ |
| Windows SDK | 10.0.22621+ | Включается в VS 2022 |
| Python | 3.11+ | https://python.org/ |
| Git | Последняя | https://git-scm.com/ |
| depot_tools | Последняя | https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html |
| NSIS | 3.x | https://nsis.sourceforge.io/ |
| CMake | 3.20+ | https://cmake.org/ |

### Дополнительные зависимости Python
```bash
pip install argon2-cffi cryptography
```

## 2. Клонирование Chromium

```bash
# Установка depot_tools
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
set PATH=%PATH%;C:\path\to\depot_tools

# Создание рабочей директории
mkdir chromium && cd chromium
fetch --nohooks chromium

# Переключение на стабильную версию
cd src
git checkout tags/131.0.6778.85

# Установка зависимостей
gclient runhooks
```

## 3. Настройка GN (Build Configuration)

```bash
# Создание директории сборки
cd src
gn gen out/Default --args='
  # Базовые настройки
  is_debug = false
  is_official_build = true
  target_os = "win"
  target_cpu = "x64"
  is_clang = true
  use_lld = true

  # ОТКЛЮЧЕНИЕ GOOGLE-СЕРВИСОВ
  safe_browsing_mode = 0
  enable_translate = false
  enable_remoting = false
  enable_reporting = false
  enable_nacl = false
  enable_widevine = false
  enable_sync = false
  enable_push_api = false

  # КРИПТОГРАФИЯ
  use_openssl = true

  # РАСШИРЕНИЯ
  enable_extensions = true
  enable_manifest_v3 = true

  # ОПТИМИЗАЦИИ
  use_lto = true
  lto_mode = "thin"

  # КАСТОМНЫЕ ФЛАГИ ANONYMUS
  anonymus_browser = true
  anonymus_kill_switch = true
  anonymus_fingerprint_protection = true
  anonymus_tracker_blocker = true
  anonymus_tor_integration = true
'
```

## 4. Интеграция исходников Anonymus Browser

```bash
# Копирование исходников
xcopy /E /I "anonymus-browser\src" "chromium\src\anonymus"

# Создание BUILD.gn файлов
# (см. файл build/anonymus BUILD.gn в проекте)
```

### BUILD.gn для модулей Anonymus Browser

```gn
# chromium/src/anonymus/BUILD.gn

source_set("anonymus_browser") {
  sources = [
    "browser/anonymus_browser_client.cc",
    "browser/anonymus_browser_client.h",
    "net/kill_switch.cc",
    "net/kill_switch.h",
    "net/network_request_interceptor.cc",
    "net/network_request_interceptor.h",
    "crypto/aes_gcm_cipher.cc",
    "crypto/aes_gcm_cipher.h",
    "crypto/argon2_key_derivation.cc",
    "crypto/argon2_key_derivation.h",
    "password_manager/password_store.cc",
    "password_manager/password_store.h",
    "fingerprint/fingerprint_generator.cc",
    "fingerprint/fingerprint_generator.h",
    "tracker_blocker/tracker_database.cc",
    "tracker_blocker/tracker_database.h",
    "ai_antiphishing/phishing_detector.cc",
    "ai_antiphishing/phishing_detector.h",
    "virtual_disk/virtual_disk.cc",
    "virtual_disk/virtual_disk.h",
    "extensions/extension_sandbox.cc",
    "extensions/extension_sandbox.h",
    "tor_integration/tor_bootstrap.cc",
    "tor_integration/tor_bootstrap.h",
  ]

  deps = [
    "//base",
    "//content/public/browser",
    "//content/public/renderer",
    "//net",
    "//url",
    "//third_party/boringssl",
    "//third_party/argon2",
  ]

  if (is_win) {
    libs += [
      "crypt32.lib",
      "ncrypt.lib",
    ]
  }
}
```

## 5. Сборка

```bash
# Полная сборка (занимает 2-6 часов)
cd chromium\src
autoninja -C out/Default chrome

# Сборка только тестов
autoninja -C out/Default anonymus_tests
```

## 6. Тестирование

```bash
# Запуск браузера
out\Default\chrome.exe

# Запуск тестов
out\Default\anonymus_tests

# Проверка блокировки Google-сервисов
# (в DevTools Console)
fetch('https://clients4.google.com/chrome/sync').then(r => console.log(r.status))
# Должно вернуть ошибку сети
```

## 7. Создание установщика

```bash
# Сборка NSIS установщика
cd anonymus-browser\installer
makensis /DOUTPUT_DIR=..\build_output anonymus_installer.nsi

# Результат: AnonymusBrowserSetup.exe
```

## 8. Цифровая подпись

```bash
# Подпись установщика (требуется сертификат)
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com ^
  build_output\AnonymusBrowserSetup.exe
```

## 9. Структура релиза

```
AnonymusBrowser-v1.0.0/
├── AnonymusBrowserSetup.exe       # Установщик
├── README.txt                      # Описание
├── LICENSE.txt                     # Лицензия
└── CHANGELOG.txt                   # История изменений
```

## 10. Отладка

### Включение логов
```bash
# Запуск с логами
chrome.exe --enable-logging --v=1 --anonymus-debug

# Логи Kill Switch
chrome.exe --anonymus-log-killswitch

# Логи фингерпринтинга
chrome.exe --anonymus-log-fingerprint
```

### DevTools
- `F12` — открытие DevTools
- `chrome://net-internals/` — сетевая диагностика
- `chrome://extensions/` — управление расширениями

---

**Важно:** Полная сборка из исходников Chromium требует значительных ресурсов
и занимает несколько часов. Для быстрого прототипирования рекомендуется
использовать предварительно собранный Chromium и интегрировать только
кастомные модули.
