#ifndef ANONYMUS_BROWSER_UI_ADAPTIVE_DPI_H_
#define ANONYMUS_BROWSER_UI_ADAPTIVE_DPI_H_

#include <cstdint>
#include <string>

#include "base/files/file_path.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

#ifdef OS_WIN
#include <windows.h>
#endif

namespace anonymus {
namespace ui {

// ============================================================
// Режимы отображения
// ============================================================
enum class DisplayMode {
  kCompact,      // < 1024px ширина (маленькие ноутбуки)
  kStandard,     // 1024-1920px (Full HD)
  kLarge,        // 1920-2560px (2K)
  kUltraHD,      // > 2560px (4K, UltraWide)
  kAuto          // Автоопределение
};

// ============================================================
// Конфигурация DPI
// ============================================================
struct DpiConfig {
  float scale_factor = 1.0f;     // 1.0 = 100%, 1.25 = 125%, etc.
  int dpi = 96;                   // Текущий DPI
  int screen_width = 1920;        // Ширина экрана
  int screen_height = 1080;       // Высота экрана
  DisplayMode mode = DisplayMode::kAuto;

  // Вычисление размера с учётом DPI
  int Scale(int value) const {
    return static_cast<int>(value * scale_factor);
  }

  float ScaleF(float value) const {
    return value * scale_factor;
  }
};

// ============================================================
// Настройки шрифтов
// ============================================================
struct FontConfig {
  int title_size = 14;
  int body_size = 13;
  int caption_size = 11;
  int ui_size = 12;
  std::string family = "Segoe UI";

  static FontConfig ForMode(DisplayMode mode, int dpi);
};

// ============================================================
// Настройки отступов
// ============================================================
struct SpacingConfig {
  int padding_small = 4;
  int padding_medium = 8;
  int padding_large = 16;
  int padding_xlarge = 24;

  int margin_window = 8;
  int margin_content = 12;

  int toolbar_height = 36;
  int tab_height = 32;
  int sidebar_width = 48;
  int statusbar_height = 24;

  static SpacingConfig ForMode(DisplayMode mode);
};

// ============================================================
// Настройки размера окна
// ============================================================
struct WindowConfig {
  int min_width = 800;
  int min_height = 600;
  int default_width = 1200;
  int default_height = 800;

  // Соотношение сторон
  float aspect_ratio = 0.0f;  // 0 = свободный

  // Табы при переполнении
  int max_visible_tabs = 10;
  bool collapse_tabs_to_menu = true;

  static WindowConfig ForMode(DisplayMode mode);
};

// ============================================================
// Адаптивный UI менеджер
// ============================================================
class AdaptiveDpiManager {
 public:
  AdaptiveDpiManager();
  ~AdaptiveDpiManager();

  AdaptiveDpiManager(const AdaptiveDpiManager&) = delete;
  AdaptiveDpiManager& operator=(const AdaptiveDpiManager&) = delete;

  // Singleton
  static AdaptiveDpiManager* GetInstance();

  // Инициализация (вызывается при старте)
  void Initialize();

  // Определение текущего DPI и режима
  DpiConfig DetectDpiConfig();

  // Принудительная установка режима
  void SetDisplayMode(DisplayMode mode);

  // Получение конфигураций
  const FontConfig& GetFontConfig() const { return font_config_; }
  const SpacingConfig& GetSpacingConfig() const { return spacing_config_; }
  const WindowConfig& GetWindowConfig() const { return window_config_; }
  const DpiConfig& GetDpiConfig() const { return dpi_config_; }

  // Масштабирование значений
  int Scale(int value) const { return dpi_config_.Scale(value); }
  float ScaleF(float value) const { return dpi_config_.ScaleF(value); }

  // Проверка: нужно ли сворачивать табы
  bool ShouldCollapseTabs(int window_width) const;

  // Получение количества видимых табов
  int GetVisibleTabCount(int window_width) const;

  // CSS для текущего режима (для WebView)
  std::string GenerateCSS() const;

  // Обработка изменения размера окна
  void OnWindowResize(int width, int height);

  // Проверка минимального размера
  gfx::Size GetMinimumWindowSize() const;

  // Сохранение/загрузка настроек
  bool SaveConfig(const base::FilePath& path);
  bool LoadConfig(const base::FilePath& path);

 private:
  // Определение режима по размеру экрана
  DisplayMode DetectMode(int screen_width, int screen_height, int dpi);

  // Адаптация конфигураций под режим
  void ApplyMode(DisplayMode mode);

  DpiConfig dpi_config_;
  FontConfig font_config_;
  SpacingConfig spacing_config_;
  WindowConfig window_config_;
  DisplayMode current_mode_ = DisplayMode::kAuto;
};

// ============================================================
// Утилиты для WinAPI
// ============================================================
class WinDpiUtil {
 public:
  // Получение DPI текущего монитора
  static int GetSystemDpi();

  // Получение DPI для конкретного окна
  static int GetWindowDpi(void* hwnd);

  // Получение размеров экрана
  static void GetScreenSize(int* width, int* height);

  // Получение количества мониторов
  static int GetMonitorCount();

  // Проверка: High DPI режим
  static bool IsHighDpi();

  // Установка DPI awareness для процесса
  static void SetProcessDpiAwareness();

  // Конвертация CSS пикселей в физические
  static int CssToPhysical(int css_pixels, int dpi);

  // Конвертация физических пикселей в CSS
  static int PhysicalToCss(int physical_pixels, int dpi);
};

// ============================================================
// Генератор CSS
// ============================================================
class AdaptiveCSS {
 public:
  // Генерация CSS переменных для текущего режима
  static std::string GenerateCSSVariables(const DpiConfig& dpi,
                                           const FontConfig& font,
                                           const SpacingConfig& spacing);

  // Генерация media queries
  static std::string GenerateMediaQueries();

  // Генерация стилей для табов
  static std::string GenerateTabStyles(const WindowConfig& window,
                                        int current_width);

  // Генерация стилей для тулбара
  static std::string GenerateToolbarStyles(const SpacingConfig& spacing);

  // Минимальные стили (для compact режима)
  static std::string GetCompactStyles();

  // Стили для 4K
  static std::string GetUltraHDStyles();
};

}  // namespace ui
}  // namespace anonymus

#endif  // ANONYMUS_BROWSER_UI_ADAPTIVE_DPI_H_
