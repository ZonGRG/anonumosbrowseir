# Anonymus Browser

## Адаптивный UI/UX

### Проблема

Пользователи работают на разных мониторах:
- Маленькие ноутбуки: 1366x768
- Full HD: 1920x1080
- 2K: 2560x1440
- 4K: 3840x2160
- UltraWide 21:9: 3440x1440

### Решение

Адаптивный UI автоматически определяет DPI и размер экрана, подстраивая все элементы.

## Режимы отображения

| Режим | Экран | DPI | Шрифты |
|-------|-------|-----|--------|
| Compact | < 1024px | 96-120 | 9-10px |
| Standard | 1024-1920px | 96-144 | 11-12px |
| Large | 1920-2560px | 144+ | 13-14px |
| UltraHD | > 2560px | 192+ | 15-16px |

## Определение DPI

```cpp
// Windows API
int dpi = GetSystemMetrics(SM_CXSCREEN);
float scale = dpi / 96.0f;

// Для конкретного окна
int dpi = GetDpiForWindow(hwnd);
```

## Масштабирование

### CSS переменные

```css
:root {
  --dpi-scale: 1.25;  /* 125% */
  --font-title: 14px;
  --font-body: 13px;
  --toolbar-height: 40px;
  --tab-height: 36px;
  --sidebar-width: 52px;
}
```

### Media queries

```css
/* Компактный режим */
@media (max-width: 1023px) {
  :root {
    --font-title: 10px;
    --toolbar-height: 28px;
  }
}

/* 4K */
@media (min-width: 2560px) {
  :root {
    --font-title: 16px;
    --toolbar-height: 44px;
  }
}
```

## Адаптация табов

### Логика

```
Если окно узкое:
  - Табы сворачиваются в выпадающий список
  - Максимум N видимых табов
  - Остальные в меню "..."

Если окно широкое:
  - Все табы видны
  - Увеличенные отступы
```

### Расчёт

```cpp
int GetVisibleTabCount(int window_width) {
  int tab_width = 150;  // px
  int available = window_width - sidebar_width - padding;
  return available / tab_width;
}
```

## Минимальный размер

```
Минимум: 800x600
Рекомендуется: 1200x800
```

Если пользователь пытается уменьшить окно меньше минимума — окно автоматически возвращается к минимальному размеру.

## Векторная графика

Все иконки используют SVG для чёткости на любых DPI:

```html
<!-- Правильно -->
<svg viewBox="0 0 24 24">
  <path d="M12 2L2 7l10 5..."/>
</svg>

<!-- Неправильно -->
<img src="icon-24x24.png">
```

## Шрифты

Шрифты подгружаются из локальных ресурсов:

```
resources/fonts/
├── SegoeUI.ttf
├── SegoeUI-Bold.ttf
└── SegoeUI-Semibold.ttf
```

НЕ используются веб-шрифты (Google Fonts и т.д.) для безопасности.

## Тестирование

### Проверка на разных DPI

```
Windows Settings → Display → Scale
- 100% (96 DPI)
- 125% (120 DPI)
- 150% (144 DPI)
- 200% (192 DPI)
```

### Проверка на разных разрешениях

```
1366x768  — Compакт
1920x1080 — Standard
2560x1440 — Large
3840x2160 — UltraHD
3440x1440 — UltraWide
```

## Реализация

См. `src/ui/adaptive_dpi.h` и `src/ui/adaptive_dpi.cc`.
