<p align="center">
  <br>
  <img src="https://raw.githubusercontent.com/Anonymus-Browser/Anonymus/main/docs/assets/logo.svg" width="120" alt="Anonymus Browser Logo">
  <br>
</p>

<h1 align="center">
  <code>ANONYMUS BROWSER</code>
</h1>

<p align="center">
  <strong>Браузер для максимальной приватности и анонимности</strong>
  <br>
  <em>Базируется на Chromium | Полная изоляция от Google | AES-256-GCM шифрование</em>
</p>

<p align="center">
  <a href="https://github.com/Anonymus-Browser/Anonymus/releases">
    <img src="https://img.shields.io/github/v/release/Anonymus-Browser/Anonymus?style=flat-square&color=00ff41" alt="Version">
  </a>
  <a href="https://github.com/Anonymus-Browser/Anonymus/actions">
    <img src="https://img.shields.io/github/actions/workflow/status/Anonymus-Browser/Anonymus/build.yml?style=flat-square&label=build" alt="Build Status">
  </a>
  <a href="https://github.com/Anonymus-Browser/Anonymus/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/Anonymus-Browser/Anonymus?style=flat-square" alt="License">
  </a>
  <a href="https://github.com/Anonymus-Browser/Anonymus/releases">
    <img src="https://img.shields.io/github/downloads/Anonymus-Browser/Anonymus/total?style=flat-square&color=00ff41" alt="Downloads">
  </a>
</p>

<p align="center">
  <a href="#-установка">Установка</a> •
  <a href="#-скриншоты">Скриншоты</a> •
  <a href="#-возможности">Возможности</a> •
  <a href="#-сборка">Сборка</a> •
  <a href="#-контрибьюция">Контрибьюция</a> •
  <a href="#-лицензия">Лицензия</a>
</p>

---

```
    ██████████████████████████████████████████████████████
    █                                                    █
    █   ██████╗ ██╗  ██╗ ██████╗ ███████╗██████╗ ███████╗ █
    █  ██╔════╝ ██║  ██║██╔═══██╗██╔════╝██╔══██╗██╔════╝ █
    █  ██║      ███████║██║   ██║███████╗██████╔╝███████╗ █
    █  ██║      ██╔══██║██║   ██║╚════██║██╔═══╝ ╚════██║ █
    █  ╚██████╗ ██║  ██║╚██████╔╝███████║██║     ███████║ █
    █   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝     ╚══════╝ █
    █              B R O W S E R                            █
    █                                                        █
    █   Приватность. Безопасность. Анонимность.              █
    █                                                        █
    ██████████████████████████████████████████████████████
```

---

## Что это?

**Anonymus Browser** — это десктопный браузер для Windows, созданный на базе open-source движка Chromium с полной изоляцией от экосистемы Google.

### Поддерживаемые ОС

| ОС | Архитектура | Статус |
|----|-------------|--------|
| Windows 10 (1809+) | x64 | ✅ Полная поддержка |
| Windows 11 | x64 | ✅ Полная поддержка |
| Windows 10/11 | ARM64 | 🔄 В разработке |
| Linux | x64 | 📋 Планируется |
| macOS | ARM64 | 📋 Планируется |

---

## Установка

### Скачивание

**Рекомендуемый способ:** скачайте последний релиз с [GitHub Releases](https://github.com/Anonymus-Browser/Anonymus/releases/latest).

```
AnonymusBrowserSetup_x64.exe  (~50 MB)
```

### Тихая установка

```cmd
AnonymusBrowserSetup_x64.exe /S /D=C:\Program Files\Anonymus Browser
```

### Portable версия

Скачайте `AnonymusPortable.zip`, распакуйте в любую папку и запустите `anonymus.exe`.

---

## Скриншоты

<p align="center">
  <em>Тёмная тема Matrix с неоновыми акцентами</em>
</p>

```
┌─────────────────────────────────────────────────────────────┐
│ ◉ ANONYMUS BROWSER  v2.1.4                    ─  □  ✕     │
├─────────────────────────────────────────────────────────────┤
│ ◀ ▶ ↻  │ 🔒 https://example.com                  │ ⚙ ⋮  │
├─────────────────────────────────────────────────────────────┤
│ [+tab1] [+tab2] [anonymous.docx ×]                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│                                                             │
│                   КОНТЕНТ СТРАНИЦЫ                          │
│                                                             │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ 🟢 Tor: Active │ 🔒 Kill Switch: ON │ 🔐 AES-256 │ v2.1.4 │
└─────────────────────────────────────────────────────────────┘
```

---

## Возможности

### Приватность и безопасность

| Функция | Описание |
|---------|----------|
| **Anti-Google** | Полное отключение Safe Browsing, Sync, Translate, Analytics |
| **Kill Switch** | Автоблокировка интернета при падении прокси |
| **Tor интеграция** | Встроенная поддержка Tor (obfs4, Snowflake) |
| **Шифрование** | AES-256-GCM + Argon2id для мастер-пароля |
| **Фингерпринтинг** | Защита Canvas, WebGL, AudioContext |
| **Трекеры** | Встроенный C++ блокировщик (не расширение!) |
| **AI Anti-фишинг** | ML-модель для обнаружения фишинговых сайтов |
| **Пароли** | Изолированный менеджер в SQLCipher |

### Технические детали

```
Движок:          Chromium (open-source, не Google Chrome)
Шифрование:      AES-256-GCM (OpenSSL)
KDF:             Argon2id (4 итерации, 64MB RAM)
БД паролей:      SQLCipher (SQLite + AES-256-CBC)
Установщик:      NSIS 3.x + Python GUI
Автообновления:  GitHub Releases API (SemVer)
UI:              Chromium Views (native C++)
Тема:            Matrix Dark (неон зелёный #00ff41)
```

---

## Сборка

### Требования

- Windows 10/11 (x64)
- Visual Studio 2022+
- Python 3.11+
- 16+ GB RAM
- 100+ GB свободного места (SSD)

### Быстрая сборка

```bash
# 1. Клонирование Chromium
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fetch --nohooks chromium
cd src && git checkout tags/131.0.6778.85
gclient runhooks

# 2. Копирование исходников Anonymus Browser
xcopy /E /I "anonymus-browser\src" "chromium\src\anonymus"

# 3. Настройка GN
gn gen out/Default --args='
  is_debug = false
  is_official_build = true
  target_os = "win"
  target_cpu = "x64"
  safe_browsing_mode = 0
  enable_translate = false
  enable_sync = false
  anonymus_browser = true
'

# 4. Сборка
autoninja -C out/Default chrome
```

Подробнее: [docs/BUILD_INSTRUCTIONS.md](docs/BUILD_INSTRUCTIONS.md)

---

## Конфигурация

### Настройки по умолчанию

```json
{
  "privacy": {
    "kill_switch": true,
    "fingerprint_protection": true,
    "tracker_blocker": true,
    "ai_antiphishing": true,
    "tor": false
  },
  "encryption": {
    "algorithm": "AES-256-GCM",
    "kdf": "Argon2id",
    "password_min_length": 12
  },
  "network": {
    "proxy": "",
    "tor_socks5": "127.0.0.1:9050",
    "dns_over_https": true
  }
}
```

### Конфигурация прокси

```
Настройки → Прокси / Tor → SOCKS5 Прокси
Адрес: 127.0.0.1:9050
```

---

## Автообновления

Браузер проверяет обновления через GitHub Releases API:

1. **Автоматически:** при запуске + каждые 4 часа
2. **Вручную:** Настройки → О браузере → Проверить обновления

```
GET https://api.github.com/repos/Anonymus-Browser/Anonymus/releases/latest
```

Подробнее: [docs/UPDATE_SYSTEM.md](docs/UPDATE_SYSTEM.md)

---

## Контрибьюция

### Ветки

| Ветка | Назначение |
|-------|-----------|
| `main` | Стабильные релизы |
| `dev` | Ночные сборки |
| `feature/*` | Новые фичи |
| `hotfix/*` | Срочные исправления |

### Conventional Commits

```
feat: добавлена поддержка Snowflake для Tor
fix: исправлена утечка куки через WebRTC
security: обновлен OpenSSL до 3.2.0
docs: обновлена инструкция по сборке
```

Подробнее: [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md)

---

## Безопасность

### Политика безопасности

Смотрите [SECURITY.md](SECURITY.md).

### Баг-баунти

Мы ценим безопасность. Если вы нашли уязвимость:

1. **НЕ** публикуйте её публично
2. Отправьте описание на security@anonymus-browser.com
3. Дождитесь подтверждения

### Награды

| Уровень | Описание | Награда |
|---------|----------|---------|
| Critical | RCE, утечка паролей | $500 - $2000 |
| High | XSS, CSRF | $100 - $500 |
| Medium | Информационные | $50 - $100 |
| Low | Косметические | Благодарность |

---

## Технологии

<p align="center">
  <img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white" alt="Python">
  <img src="https://img.shields.io/badge/Chromium-4285F4?style=for-the-badge&logo=googlechrome&logoColor=white" alt="Chromium">
  <img src="https://img.shields.io/badge/OpenSSL-721817?style=for-the-badge&logo=openssl&logoColor=white" alt="OpenSSL">
  <img src="https://img.shields.io/badge/NSIS-23A24B?style=for-the-badge" alt="NSIS">
  <img src="https://img.shields.io/badge/Tor-7D4698?style=for-the-badge&logo=torproject&logoColor=white" alt="Tor">
</p>

---

## Статистика

![GitHub stars](https://img.shields.io/github/stars/Anonymus-Browser/Anonymus?style=social)
![GitHub forks](https://img.shields.io/github/forks/Anonymus-Browser/Anonymus?style=social)
![GitHub issues](https://img.shields.io/github/issues/Anonymus-Browser/Anonymus)

---

## Лицензия

Этот проект лицензирован по [BSD-3-Clause](LICENSE) — та же лицензия, что и Chromium.

```
Copyright (c) 2026 Anonymus Security
All rights reserved.
```

---

<p align="center">
  <strong>Приватность — это право, а не привилегия.</strong>
  <br>
  <sub>Создано с ❤️ для тех, кто ценит свою свободу</sub>
</p>
