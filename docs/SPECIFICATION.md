# Anonymus Browser — Техническая Спецификация v1.0

## 1. Обзор проекта

**Anonymus Browser** — десктопный браузер для Windows, базирующийся на open-source движке Chromium, спроектированный для максимальной приватности и анонимности. Браузер полностью изолирован от экосистемы Google и предоставляет встроенные средства шифрования, скрытия IP, анти-фингерпринтинга и менеджмента секретов.

---

## 2. Архитектура проекта

```
anonymus-browser/
├── src/
│   ├── browser/              # Основной процесс браузера (UI, вкладки, рендеринг)
│   │   ├── anonymus_browser_main.cc
│   │   ├── anonymus_browser_client.h/cc
│   │   ├── anonymus_content_browser_client.h/cc
│   │   └── resources/        # Иконки, ресурсы
│   ├── net/                  # Сетевой стек (прокси, Kill Switch, Tor)
│   │   ├── proxy_resolver.h/cc
│   │   ├── kill_switch.h/cc
│   │   ├── tor_bootstrap.h/cc
│   │   ├── cookie_isolation.h/cc
│   │   └── network_request_interceptor.h/cc
│   ├── crypto/               # Шифрование данных
│   │   ├── aes_gcm_cipher.h/cc
│   │   ├── argon2_key_derivation.h/cc
│   │   └── encrypted_database.h/cc
│   ├── password_manager/     # Локальный менеджер паролей
│   │   ├── password_store.h/cc
│   │   ├── password_generator.h/cc
│   │   └── credential_encryption.h/cc
│   ├── fingerprint/          # Генератор и защита от фингерпринтинга
│   │   ├── fingerprint_generator.h/cc
│   │   ├── canvas_noise.h/cc
│   │   ├── webgl_spoofing.h/cc
│   │   └── audio_context_noise.h/cc
│   ├── tracker_blocker/      # Встроенный блокировщик трекеров (C++)
│   │   ├── tracker_database.h/cc
│   │   ├── request_classifier.h/cc
│   │   └── adblock_engine.h/cc
│   ├── ai_antiphishing/      # AI анти-фишинг
│   │   ├── phishing_detector.h/cc
│   │   ├── url_analyzer.h/cc
│   │   └── ml_model_loader.h/cc
│   ├── virtual_disk/         # Виртуальный зашифрованный диск
│   │   ├── veracrypt_container.h/cc
│   │   ├── disk_mounter.h/cc
│   │   └── secure_wipe.h/cc
│   ├── extensions/           # Поддержка расширений
│   │   ├── extension_sandbox.h/cc
│   │   ├── crx_loader.h/cc
│   │   └── webstore_proxy.h/cc
│   └── tor_integration/      # Интеграция с Tor
│       ├── tor_circuit_manager.h/cc
│       ├── bridge_config.h/cc
│       └── onion_resolver.h/cc
├── python/
│   ├── encrypt_database.py   # Шифрование БД паролей
│   ├── generate_keys.py      # Генерация ключей
│   ├── build_tool.py         # Скрипт сборки
│   └── test_crypto.py        # Юнит-тесты криптографии
├── installer/
│   ├── anonymus_installer.nsi  # NSIS-скрипт установщика
│   └── resources/              # Иконки установщика
├── build/
│   ├── config.gn             # GN-конфигурация сборки
│   └── build.bat             # Скрипт сборки
├── ui/
│   └── themes/
│       ├── dark_matrix.css   # CSS тема "Matrix"
│       └── settings.html     # Макет страницы настроек
├── config/
│   ├── tracker_list.txt      # База URL трекеров
│   └── tor_bridges.json      # Конфигурация Tor-мостов
└── docs/
    ├── SPECIFICATION.md      # Этот файл
    └── BUILD_INSTRUCTIONS.md # Инструкция по сборке
```

---

## 3. Модульная архитектура

### 3.1 browser/ — Основной процесс

**Ответственность:** Инициализация браузера, управление окнами, вкладками, UI.

Ключевые компоненты:
- `AnonymusBrowserMain` — точка входа, замена `ChromeMain`
- `AnonymusBrowserClient` — замена `ChromeBrowserClient`, отключает все Google-сервисы
- `AnonymusContentBrowserClient` — перехват навигации, инжекция скриптов фингерпринтинга

**Anti-Google патчи (deprecated_scripts.h):**
```cpp
// Полный запрет обращений к Google-сервисам
const char* kBlockedGoogleDomains[] = {
    "safebrowsing.googleapis.com",
    "clients4.google.com",
    "www.google-analytics.com",
    "translate.googleapis.com",
    "push.clients.google.com",
    "clients2.google.com",
    "www.google.com/save",
    "metadata.google.com",
    "dl.google.com",
    "update.googleapis.com"
};
```

### 3.2 net/ — Сетевой стек

**Ответственность:** Прокси, Kill Switch, Tor-интеграция, изоляция куки, перехват запросов.

#### Kill Switch (`kill_switch.h`)
```cpp
class KillSwitch {
 public:
  enum class State { ACTIVE, PROXY_DOWN, INTERNET_BLOCKED };

  // Проверка доступности прокси каждые N секунд
  void StartProxyHealthCheck(base::TimeDelta interval);

  // Полная блокировка сетевого стека при падении прокси
  void BlockAllNetworkTraffic();

  // Разблокировка при восстановлении прокси
  void RestoreNetworkTraffic();

  State GetState() const;

 private:
  void OnProxyHealthCheckFailed();
  void OnProxyHealthCheckRestored();
  void NotifyAllRenderers(bool blocked);

  State state_ = State::ACTIVE;
  raw_ptr<net::NetworkQualityEstimator> nqe_;
  base::RepeatingTimer health_check_timer_;
  std::vector<raw_ptr<content::RenderProcessHost>> renderer_hosts_;
};
```

#### Перехват сетевых запросов (`network_request_interceptor.h`)
```cpp
class NetworkRequestInterceptor : public net::URLRequestInterceptor {
 public:
  // Перехват каждого исходящего запроса
  int OnBeforeURLRequest(net::URLRequest* request,
                        net::CompletionOnceCallback callback,
                        GURL* new_url) override;

  // Удаление приватных заголовков
  void StripPrivateHeaders(net::URLRequest* request);

  // Подмена User-Agent для Chrome Web Store
  void SpoofUserAgentForWebStore(net::URLRequest* request);

 private:
  void BlockGoogleTelemetry(net::URLRequest* request);
  void StripClientIdHeaders(net::URLRequest* request);
  void InjectAnonymusHeaders(net::URLRequest* request);
};
```

### 3.3 crypto/ — Шифрование

**Ответственность:** AES-256-GCM шифрование, генерация ключей через Argon2id, управление зашифрованными БД.

#### Шифрование AES-256-GCM (`aes_gcm_cipher.h`)
```cpp
class AesGcmCipher {
 public:
  static constexpr size_t kKeySize = 32;    // 256 bits
  static constexpr size_t kNonceSize = 12;  // 96 bits
  static constexpr size_t kTagSize = 16;    // 128 bits

  // Шифрование с authenticated encryption
  std::vector<uint8_t> Encrypt(
      base::span<const uint8_t> plaintext,
      base::span<const uint8_t> key,
      base::span<const uint8_t> nonce);

  // Дешифрование с проверкой тега
  bool Decrypt(
      base::span<const uint8_t> ciphertext_with_tag,
      base::span<const uint8_t> key,
      base::span<const uint8_t> nonce,
      std::vector<uint8_t>& plaintext_out);
};
```

#### Генерация ключей Argon2id (`argon2_key_derivation.h`)
```cpp
class Argon2KeyDerivation {
 public:
  struct Params {
    uint32_t time_cost = 4;        // Итерации
    uint32_t memory_cost = 65536;  // 64 MB памяти
    uint32_t parallelism = 4;      // Потоки
    size_t hash_length = 32;       // Длина ключа
  };

  // Генерация ключа из мастер-пароля
  std::vector<uint8_t> DeriveKey(
      const std::string& master_password,
      base::span<const uint8_t> salt);

  // Генерация криптографически стойкого соли
  std::vector<uint8_t> GenerateSalt(size_t length = 32);
};
```

### 3.4 password_manager/ — Менеджер паролей

**Ответственность:** Хранение, генерация и шифрование учётных данных.

- Полная изоляция от Chromium Password Manager
- Куки и пароли хранятся в SQLCipher
- При входе в систему: запрашивается мастер-пароль
- Пароли дешифруются только в оперативной памяти
- Автозаполнение работает через content script инъекцию

### 3.5 fingerprint/ — Защита от фингерпринтинга

**Ответственность:** Генерация и подмена отпечатков Canvas, WebGL, AudioContext, шрифтов.

#### Генератор отпечатков (`fingerprint_generator.h`)
```cpp
class FingerprintGenerator {
 public:
  // Генерация уникального отпечатка сессии
  Fingerprint GenerateSessionFingerprint();

  // Подмена Canvas fingerprint
  void SpoofCanvasFingerprint(content::RenderFrameHost* frame);

  // Подмена WebGL fingerprint
  void SpoofWebGLFingerprint(content::RenderFrameHost* frame);

  // Шум AudioContext
  void InjectAudioContextNoise(content::RenderFrameHost* frame);

  // Список установленных шрифтов
  std::vector<std::string> GenerateFontFingerprint();

 private:
  uint64_t session_seed_;
  std::mt19937_64 rng_;
};
```

**Стратегии защиты:**
1. **Canvas Noise** — добавление незаметного шума (±1px) в canvas-рисунки
2. **WebGL Spoofing** — подмена `UNMASKED_VENDOR_WEBGL` и `UNMASKED_RENDERER_WEBGL`
3. **AudioContext** — добавление микрошума в `createOscillator()` и `AnalyserNode`
4. **Font Enumeration** — блокировка APIs обнаружения шрифтов
5. **Navigator Properties** — подмена `navigator.hardwareConcurrency`, `navigator.deviceMemory`

### 3.6 tracker_blocker/ — Блокировщик трекеров

**Ответственность:** Блокировка рекламы и трекеров на уровне C++ (не расширение).

```cpp
class TrackerBlocker {
 public:
  // Инициализация с загрузкой базы трекеров
  void Initialize(const base::FilePath& tracker_list_path);

  // Классификация URL:是否 трекер
  BlockDecision ClassifyRequest(const GURL& url);

  enum class BlockDecision {
    ALLOW,
    BLOCK_TRACKER,
    BLOCK_AD,
    BLOCK_FINGERPRINT,
    ALLOW_NEUTRAL
  };

 private:
  void LoadTrackerDatabase(base::FilePath path);
  bool MatchesCosmeticFilter(const GURL& url);
  bool MatchesNetworkFilter(const GURL& url);

  // Aho-Corasick для быстрого поиска подстрок
  AhoCorasickMatcher url_matcher_;
  base::flat_set<std::string> blocked_domains_;
  base::flat_set<std::string> blocked_ad_urls_;
};
```

**Интеграция с network stack:**
- Перехват через `content::NetworkService` 
- Блокировка на уровне `net::URLRequest`
- Косметическая фильтрация через注入 CSS/JS в renderer

### 3.7 ai_antiphishing/ — AI Антифишинг

**Ответственность:** Обнаружение фишинговых сайтов с помощью ML-модели.

```cpp
class PhishingDetector {
 public:
  // Загрузка ONNX-модели
  bool LoadModel(const base::FilePath& model_path);

  // Анализ URL и HTML-контента
  PhishingResult AnalyzeUrl(const GURL& url, const std::string& page_content);

  struct PhishingResult {
    bool is_phishing;
    float confidence;         // 0.0 - 1.0
    std::string threat_type;  // "credential_theft", "malware", "scam"
    std::string explanation;
  };

 private:
  std::vector<float> ExtractFeatures(const GURL& url);
  std::unique_ptr<onnxruntime::OrtSession> session_;
};
```

**Извлекаемые признаки:**
- Длина URL, количество поддоменов
- Наличие IP-адреса вместо домена
- Использование HTTPS vs HTTP
- Анализ домена (возраст, SSL-сертификат)
- Похожесть домена на популярные сайты (Levenshtein distance)
- Наличие подозрительных паттернов в URL

### 3.8 virtual_disk/ — Виртуальный зашифрованный диск

**Ответственность:** Монтирование контейнера для хранения истории, куки, Local Storage.

```cpp
class VirtualDisk {
 public:
  // Создание нового контейнера
  bool CreateContainer(const base::FilePath& path, uint64_t size_mb);

  // Монтирование контейнера (только во время работы браузера)
  bool MountContainer(const base::FilePath& path,
                      const std::string& password);

  // Размонтирование и зачистка
  bool UnmountAndWipe();

  // Безопасное удаление (Multi-pass wipe)
  bool SecureWipeFile(const base::FilePath& path);

 private:
  void WipeMemory();
  base::FilePath mounted_path_;
  bool is_mounted_;
};
```

### 3.9 extensions/ — Поддержка расширений

**Ответственность:** Загрузка .crx, песочница расширений, прокси Chrome Web Store.

**Песочница расширений:**
- Расширения НЕ имеют доступа к зашифрованному хранилищу
- HTTPOnly куки изолированы от расширений
- Content Script не может читать chrome.storage.sync
- WebRequest API ограничена: запрет на чтение тел запросов

### 3.10 tor_integration/ — Tor

**Ответственность:** Встроенная поддержка Tor-сети.

- Автоматическая загрузка Tor-мостов
- Управление circuit-ами
- SOCKS5 прокси на `127.0.0.1:9050`
- Поддержка obfs4, Snowflake, meek-azure
- Автоматическое переключение circuit каждые 10 минут

---

## 4. Стек технологий

| Компонент | Технология |
|-----------|-----------|
| Движок | Chromium (последний stable) |
| Основной язык | C++17/20 |
| UI | Chromium Views (native C++) |
| Шифрование | OpenSSL (AES-256-GCM), Argon2id |
| БД паролей | SQLCipher (SQLite + AES-256-CBC) |
| Скрипты сборки | Python 3.11+, GN + Ninja |
| Установщик | NSIS 3.x |
| AI модель | ONNX Runtime |
| Tor | луковый маршрутизатор (TOR) |

---

## 5. Сценарии использования

### 5.1 Первый запуск
1. Пользователь видит экран настройки с логотипом "Маска Анонимуса"
2. Запрос создания мастер-пароля (минимум 12 символов)
3. Генерация соли + Argon2id → ключ шифрования
4. Создание зашифрованного контейнера (виртуальный диск)
5. Предложение настроить прокси/Tor

### 5.2 Обычная работа
1. При каждом запуске: запрос мастер-пароля
2. Монтирование виртуального диска
3. Загрузка зашифрованной БД паролей в память
4. Все сетевые запросы проходят через прокси + Kill Switch
5. Фингерпринт генерируется заново для каждой сессии
6. При закрытии: размонтирование + зачистка памяти

### 5.3 Kill Switch срабатывание
1. Прокси перестал отвечать (Health Check: 3 попытки × 5 сек)
2. Kill Switch → `BlockAllNetworkTraffic()`
3. Уведомление пользователя в UI
4. Все вкладки показывают "Интернет заблокирован"
5. При восстановлении прокси → автоматическая разблокировка

---

## 6. Безопасностные гарантии

| Угроза | Мера защиты |
|--------|------------|
| Утечка IP | Kill Switch + прокси + Tor |
| Утечка куки | SQLCipher + изоляция от расширений |
| Фингерпринтинг | Шум Canvas/WebGL/Audio + подмена параметров |
| Брутфорс мастер-пароля | Argon2id (4 итерации, 64MB RAM) |
| Утечка паролей | AES-256-GCM + виртуальный диск |
| Трекинг Google | Вырезанные сервисы + блокировка телеметрии |
| MITM-атака | Certificate pinning для критических доменов |
| Аналитика сайтов | Встроенный блокировщик трекеров |
