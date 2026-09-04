# Контрибьюция в Anonymus Browser

Спасибо за интерес к проекту! Этот документ описывает, как внести свой вклад.

## Быстрый старт

1. Fork репозитория
2. Клонируйте ваш fork
3. Создайте ветку для фичи
4. Внесите изменения
5. Отправьте Pull Request

```bash
# Клонирование
git clone https://github.com/YOUR_USERNAME/Anonymus.git
cd Anonymus

# Создание ветки
git checkout -b feature/my-feature

# Коммит
git add .
git commit -m "feat: добавлена моя фича"

# Пуш
git push origin feature/my-feature
```

## Ветки

| Ветка | Описание |
|-------|----------|
| `main` | Стабильные релизы (только проверенные изменения) |
| `dev` | Ежедневные сборки и тесты |
| `feature/*` | Новые функции |
| `hotfix/*` | Срочные исправления безопасности |

## Conventional Commits

Используйте формат коммитов:

```
<type>: <description>

[optional body]

[optional footer]
```

### Типы

| Тип | Описание |
|-----|----------|
| `feat` | Новая функция |
| `fix` | Исправление бага |
| `security` | Исправление безопасности |
| `docs` | Документация |
| `style` | Форматирование (не влияет на код) |
| `refactor` | Рефакторинг |
| `perf` | Оптимизация производительности |
| `test` | Тесты |
| `chore` | Сборка, CI/CD |

### Примеры

```bash
# Новая фича
git commit -m "feat: добавлена поддержка Snowflake для Tor"

# Исправление бага
git commit -m "fix: исправлена работа Kill Switch при роуминге"

# Исправление безопасности
git commit -m "security: обновлён OpenSSL до 3.2.1"

# Документация
git commit -m "docs: обновлена инструкция по сборке"
```

## Стиль кода

### C++

- Следуйте [Chromium C++ Style Guide](https://chromium.googlesource.com/chromium/src/+/main/styleguide/c++/c++.md)
- Используйте `base::` классы вместо STL
- Именование: `CamelCase` для классов, `snake_case` для переменных

```cpp
// Правильно
class PasswordManager {
 public:
  void StorePassword(const GURL& origin,
                     const std::string& username,
                     const std::string& password);

 private:
  std::unique_ptr<sql::Database> db_;
};

// Неправильно
class password_manager {
  void store_password(GURL origin, string username, string password) {
    // ...
  }
};
```

### Python

- Форматирование: [Black](https://black.readthedocs.io/)
- Линтинг: [Ruff](https://docs.astral.sh/ruff/)
- Типизация: [mypy](https://mypy-lang.org/)

```python
# Правильно
def encrypt_data(plaintext: bytes, key: bytes) -> bytes:
    """Шифрование данных AES-256-GCM."""
    cipher = AESGCM(key)
    nonce = os.urandom(12)
    return nonce + cipher.encrypt(nonce, plaintext, None)

# Неправильно
def encrypt_data(plaintext, key):
    cipher = AESGCM(key)
    nonce = os.urandom(12)
    return nonce + cipher.encrypt(nonce, plaintext, None)
```

## Тесты

### Обязательные тесты

Все изменения должны включать тесты:

```bash
# Python тесты
python python/test_crypto.py

# C++ тесты (после сборки)
out/Default/anonymus_tests
```

### Покрытие кода

Минимальное покрытие: 80% для нового кода.

## Pull Request

### Требования к PR

1. **Описание** — что делает PR и почему
2. **Тесты** — PR должен включать тесты
3. **Документация** — обновить документацию при необходимости
4. **Стиль** — соответствовать стилю проекта
5. **Одна фича** — один PR = одна фича/исправление

### Шаблон PR

```markdown
## Описание
Краткое описание изменений

## Тип изменений
- [ ] Новая фича
- [ ] Исправление бага
- [ ] Исправление безопасности
- [ ] Рефакторинг
- [ ] Документация

## Как тестировать
1. Шаг первый
2. Шаг второй

## Скриншоты (если применимо)
```

## Лицензия

Все внесённые изменения лицензируются по BSD-3-Clause.

## Контакты

- Email: contributors@anonymus-browser.com
- Discord: https://discord.gg/anonymus
