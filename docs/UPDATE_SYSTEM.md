# Система автообновлений

## Обзор

Anonymus Browser использует GitHub Releases API для проверки и скачивания обновлений. Браузер НЕ использует собственный сервер — все релизы хранятся в GitHub.

## Архитектура

```
┌─────────────────────────────────────────────────────────────┐
│                    Anonymus Browser                         │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐    ┌──────────────┐    ┌───────────────┐  │
│  │ Update      │───>│ GitHub API   │───>│ Download      │  │
│  │ Checker     │    │ Client       │    │ Manager       │  │
│  └─────────────┘    └──────────────┘    └───────────────┘  │
│         │                                      │            │
│         v                                      v            │
│  ┌─────────────┐    ┌──────────────┐    ┌───────────────┐  │
│  │ Version     │    │ Rate Limit   │    │ SHA-256       │  │
│  │ Comparator  │    │ Manager      │    │ Verifier      │  │
│  └─────────────┘    └──────────────┘    └───────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                              v
                    ┌──────────────────┐
                    │  GitHub Releases  │
                    │  api.github.com   │
                    └──────────────────┘
```

## Эндпоинты

### Проверка последнего релиза

```
GET https://api.github.com/repos/Anonymus-Browser/Anonymus/releases/latest
```

### Заголовки

```
Accept: application/vnd.github+json
User-Agent: AnonymusBrowser/2.1.3 (Windows; x64)
X-GitHub-Api-Version: 2022-11-28
Authorization: Bearer <token> (опционально)
```

## Формат ответа

```json
{
  "tag_name": "v2.1.4",
  "name": "Anonymus Browser v2.1.4",
  "body": "## Исправления\n- Исправлена утечка куки через WebRTC",
  "assets": [
    {
      "name": "AnonymusBrowserSetup_x64.exe",
      "browser_download_url": "https://github.com/.../releases/download/v2.1.4/AnonymusBrowserSetup_x64.exe",
      "size": 52428800,
      "digest": "sha256:9f86d081884c7d659a2feaa0c55ad015..."
    },
    {
      "name": "checksums_x64.txt",
      "browser_download_url": "https://github.com/.../checksums_x64.txt"
    }
  ],
  "prerelease": false,
  "published_at": "2026-09-01T10:30:00Z"
}
```

## Логика проверки

### Когда проверять

| Событие | Действие |
|---------|----------|
| Запуск браузера | Проверка |
| Каждые 4 часа | Автопроверка |
| Ручная проверка | Настройки → О браузере |
| После обновления | Проверка новой версии |

### Алгоритм

```
1. Получить текущую версию из реестра
2. Отправить GET /releases/latest
3. Парсинг JSON
4. Сравнение версий (SemVer)
5. Если remote > local:
   a. Показать уведомление
   b. Дождаться ответа пользователя
   c. Если согласен → скачать
   d. Проверить SHA-256
   e. Установить
```

## SemVer сравнение

```cpp
// Пример: "2.1.3" < "2.1.4"
int CompareVersions(const std::string& a, const std::string& b) {
  SemVer ver_a = SemVer::Parse(a);
  SemVer ver_b = SemVer::Parse(b);
  return ver_a.Compare(ver_b);
}
```

### Приоритеты

```
MAJOR > MINOR > PATCH > prerelease

2.0.0 > 1.9.9
2.1.0 > 2.0.9
2.1.4 > 2.1.3
2.1.4 > 2.1.4-beta.1
2.1.4-beta.2 > 2.1.4-beta.1
```

## Поведение при отказе

Если пользователь отказывается от обновления:

1. **Лог:** `[UPDATE] User declined update v2.1.4`
2. **Отключение функций:** Автозаполнение паролей отключается
3. **Баннер:** Появляется неубираемый баннер в правом углу
4. **Звук:** Каждые 15 минут системный звук
5. **Мигание:** Через 30 минут баннер начинает мигать

### Восстановление функций

Функции восстанавливаются ТОЛЬКО после обновления.

## Rate Limits

GitHub API имеет лимиты:

| Тип | Лимит |
|-----|-------|
| Без токена | 60 запросов/час на IP |
| С токеном | 5000 запросов/час |

### Обработка лимитов

```
Если HTTP 403:
  1. Проверить X-RateLimit-Remaining
  2. Если 0 → ждать до X-RateLimit-Reset
  3. Показать пользователю: "Обновления временно недоступны"

Если HTTP 500:
  1. Молча ждать 1 час
  2. Повторить попытку
```

## Безопасность скачивания

### HTTPS

Все запросы используют HTTPS с TLS 1.2+.

### SSL Pinning

Проверка сертификата GitHub:

```
SHA-256: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX=
```

### Проверка хеша

```
1. Скачать checksums.txt
2. Вычислить SHA-256 скачанного .exe
3. Сравнить с checksums.txt
4. Если НЕ совпадает → УДАЛИТЬ файл
```

## Установка обновления

### Флаги установщика

```
AnonymusBrowserSetup_x64.exe /UPDATE=1 /VERYSILENT /SUPPRESSMSGBOXES
```

### Что обновляется

- `anonymus.exe` — основной exe
- `*.dll` — библиотеки
- `locales/` — локализации

### Что НЕ обновляется

- `%APPDATA%\Anonymus\` — профиль пользователя
- Пароли — зашифрованы в SQLCipher
- Настройки — сохраняются

## Проверка после обновления

```
1. Запуск Anonymus.exe
2. Проверка версии в реестре
3. Показ страницы: "Обновлено до v2.1.4"
4. Загрузка Release Notes с GitHub
```

## Отладка

### Логи

```
[Update] Initialized, current version: 2.1.3
[Update] Checking for updates...
[Update] Auto-check: update available
[Update] Downloading version: 2.1.4
[Update] Download complete, verifying checksum...
[Update] Checksum verified, applying update...
```

### Ручная проверка

```
chrome://settings/about
→ "Проверить обновления"
```

### Сброс состояния

```
Удалить ключ реестра:
HKEY_CURRENT_USER\Software\Anonymus\UpdateDeclined
```
