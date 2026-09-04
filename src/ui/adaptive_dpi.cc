#include "anonymus/ui/adaptive_dpi.h"

#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "ui/gfx/geometry/rect.h"

#ifdef OS_WIN
#include <shellscalingapi.h>
#include <windows.h>
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "user32.lib")
#endif

namespace anonymus {
namespace ui {

// ============================================================
// FontConfig
// ============================================================

FontConfig FontConfig::ForMode(DisplayMode mode, int dpi) {
  FontConfig config;
  float dpi_scale = dpi / 96.0f;

  switch (mode) {
    case DisplayMode::kCompact:
      config.title_size = static_cast<int>(10 * dpi_scale);
      config.body_size = static_cast<int>(9 * dpi_scale);
      config.caption_size = static_cast<int>(8 * dpi_scale);
      config.ui_size = static_cast<int>(9 * dpi_scale);
      config.family = "Segoe UI";
      break;

    case DisplayMode::kStandard:
      config.title_size = static_cast<int>(12 * dpi_scale);
      config.body_size = static_cast<int>(11 * dpi_scale);
      config.caption_size = static_cast<int>(10 * dpi_scale);
      config.ui_size = static_cast<int>(11 * dpi_scale);
      config.family = "Segoe UI";
      break;

    case DisplayMode::kLarge:
      config.title_size = static_cast<int>(14 * dpi_scale);
      config.body_size = static_cast<int>(13 * dpi_scale);
      config.caption_size = static_cast<int>(11 * dpi_scale);
      config.ui_size = static_cast<int>(12 * dpi_scale);
      config.family = "Segoe UI";
      break;

    case DisplayMode::kUltraHD:
      config.title_size = static_cast<int>(16 * dpi_scale);
      config.body_size = static_cast<int>(15 * dpi_scale);
      config.caption_size = static_cast<int>(13 * dpi_scale);
      config.ui_size = static_cast<int>(14 * dpi_scale);
      config.family = "Segoe UI";
      break;

    case DisplayMode::kAuto:
      config = ForMode(DisplayMode::kStandard, dpi);
      break;
  }

  return config;
}

// ============================================================
// SpacingConfig
// ============================================================

SpacingConfig SpacingConfig::ForMode(DisplayMode mode) {
  SpacingConfig config;

  switch (mode) {
    case DisplayMode::kCompact:
      config.padding_small = 2;
      config.padding_medium = 4;
      config.padding_large = 8;
      config.padding_xlarge = 12;
      config.margin_window = 4;
      config.margin_content = 6;
      config.toolbar_height = 28;
      config.tab_height = 24;
      config.sidebar_width = 36;
      config.statusbar_height = 20;
      break;

    case DisplayMode::kStandard:
      config.padding_small = 4;
      config.padding_medium = 8;
      config.padding_large = 16;
      config.padding_xlarge = 24;
      config.margin_window = 8;
      config.margin_content = 12;
      config.toolbar_height = 36;
      config.tab_height = 32;
      config.sidebar_width = 48;
      config.statusbar_height = 24;
      break;

    case DisplayMode::kLarge:
      config.padding_small = 6;
      config.padding_medium = 12;
      config.padding_large = 20;
      config.padding_xlarge = 32;
      config.margin_window = 12;
      config.margin_content = 16;
      config.toolbar_height = 40;
      config.tab_height = 36;
      config.sidebar_width = 52;
      config.statusbar_height = 28;
      break;

    case DisplayMode::kUltraHD:
      config.padding_small = 8;
      config.padding_medium = 16;
      config.padding_large = 24;
      config.padding_xlarge = 40;
      config.margin_window = 16;
      config.margin_content = 20;
      config.toolbar_height = 44;
      config.tab_height = 40;
      config.sidebar_width = 56;
      config.statusbar_height = 32;
      break;

    case DisplayMode::kAuto:
      config = ForMode(DisplayMode::kStandard);
      break;
  }

  return config;
}

// ============================================================
// WindowConfig
// ============================================================

WindowConfig WindowConfig::ForMode(DisplayMode mode) {
  WindowConfig config;

  switch (mode) {
    case DisplayMode::kCompact:
      config.min_width = 640;
      config.min_height = 480;
      config.default_width = 1024;
      config.default_height = 600;
      config.max_visible_tabs = 5;
      config.collapse_tabs_to_menu = true;
      break;

    case DisplayMode::kStandard:
      config.min_width = 800;
      config.min_height = 600;
      config.default_width = 1200;
      config.default_height = 800;
      config.max_visible_tabs = 10;
      config.collapse_tabs_to_menu = true;
      break;

    case DisplayMode::kLarge:
      config.min_width = 1024;
      config.min_height = 768;
      config.default_width = 1600;
      config.default_height = 900;
      config.max_visible_tabs = 15;
      config.collapse_tabs_to_menu = false;
      break;

    case DisplayMode::kUltraHD:
      config.min_width = 1280;
      config.min_height = 720;
      config.default_width = 1920;
      config.default_height = 1080;
      config.max_visible_tabs = 20;
      config.collapse_tabs_to_menu = false;
      break;

    case DisplayMode::kAuto:
      config = ForMode(DisplayMode::kStandard);
      break;
  }

  return config;
}

// ============================================================
// AdaptiveDpiManager
// ============================================================

AdaptiveDpiManager::AdaptiveDpiManager() = default;

AdaptiveDpiManager::~AdaptiveDpiManager() = default;

// static
AdaptiveDpiManager* AdaptiveDpiManager::GetInstance() {
  static base::NoDestructor<AdaptiveDpiManager> instance;
  return instance.get();
}

void AdaptiveDpiManager::Initialize() {
  dpi_config_ = DetectDpiConfig();
  ApplyMode(dpi_config_.mode);

  LOG(INFO) << "[DPI] Initialized: "
            << dpi_config_.screen_width << "x"
            << dpi_config_.screen_height
            << " DPI=" << dpi_config_.dpi
            << " Scale=" << dpi_config_.scale_factor
            << " Mode=" << static_cast<int>(dpi_config_.mode);
}

DpiConfig AdaptiveDpiManager::DetectDpiConfig() {
  DpiConfig config;

#ifdef OS_WIN
  WinDpiUtil::GetScreenSize(&config.screen_width, &config.screen_height);
  config.dpi = WinDpiUtil::GetSystemDpi();
  config.scale_factor = config.dpi / 96.0f;
  config.mode = DetectMode(config.screen_width, config.screen_height, config.dpi);
#else
  config.screen_width = 1920;
  config.screen_height = 1080;
  config.dpi = 96;
  config.scale_factor = 1.0f;
  config.mode = DisplayMode::kStandard;
#endif

  return config;
}

void AdaptiveDpiManager::SetDisplayMode(DisplayMode mode) {
  current_mode_ = mode;
  ApplyMode(mode);
  LOG(INFO) << "[DPI] Display mode set to: " << static_cast<int>(mode);
}

bool AdaptiveDpiManager::ShouldCollapseTabs(int window_width) const {
  if (!window_config_.collapse_tabs_to_menu) return false;

  // Ширина одной вкладки ≈ 120-200px
  int tab_width = 150;
  int available_width = window_width - spacing_config_.sidebar_width -
                        spacing_config_.padding_large * 2;

  return (available_width / tab_width) < window_config_.max_visible_tabs;
}

int AdaptiveDpiManager::GetVisibleTabCount(int window_width) const {
  int tab_width = 150;
  int available_width = window_width - spacing_config_.sidebar_width -
                        spacing_config_.padding_large * 2;
  int count = available_width / tab_width;

  return std::max(1, std::min(count, window_config_.max_visible_tabs));
}

std::string AdaptiveDpiManager::GenerateCSS() const {
  return AdaptiveCSS::GenerateCSSVariables(
      dpi_config_, font_config_, spacing_config_);
}

void AdaptiveDpiManager::OnWindowResize(int width, int height) {
  // Проверка минимального размера
  gfx::Size min_size = GetMinimumWindowSize();

  if (width < min_size.width() || height < min_size.height()) {
    LOG(WARNING) << "[DPI] Window too small: " << width << "x" << height;
  }

  // Обновление конфигурации табов
  int visible_tabs = GetVisibleTabCount(width);
  LOG(INFO) << "[DPI] Visible tabs: " << visible_tabs;
}

gfx::Size AdaptiveDpiManager::GetMinimumWindowSize() const {
  return gfx::Size(window_config_.min_width, window_config_.min_height);
}

DisplayMode AdaptiveDpiManager::DetectMode(int screen_width,
                                            int screen_height,
                                            int dpi) {
  if (screen_width < 1024 || screen_height < 768) {
    return DisplayMode::kCompact;
  } else if (screen_width < 1920) {
    return DisplayMode::kStandard;
  } else if (screen_width < 2560) {
    return DisplayMode::kLarge;
  } else {
    return DisplayMode::kUltraHD;
  }
}

void AdaptiveDpiManager::ApplyMode(DisplayMode mode) {
  font_config_ = FontConfig::ForMode(mode, dpi_config_.dpi);
  spacing_config_ = SpacingConfig::ForMode(mode);
  window_config_ = WindowConfig::ForMode(mode);
}

// ============================================================
// WinDpiUtil
// ============================================================

int WinDpiUtil::GetSystemDpi() {
#ifdef OS_WIN
  HDC hdc = GetDC(NULL);
  int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(NULL, hdc);
  return dpi;
#else
  return 96;
#endif
}

int WinDpiUtil::GetWindowDpi(void* hwnd) {
#ifdef OS_WIN
  // Windows 10 1607+
  typedef UINT(WINAPI* GetDpiForWindowFunc)(HWND);
  static auto func = reinterpret_cast<GetDpiForWindowFunc>(
      GetProcAddress(GetModuleHandle(L"user32.dll"), "GetDpiForWindow"));

  if (func) {
    return func(static_cast<HWND>(hwnd));
  }

  // Fallback
  HDC hdc = GetDC(static_cast<HWND>(hwnd));
  int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(static_cast<HWND>(hwnd), hdc);
  return dpi;
#else
  return 96;
#endif
}

void WinDpiUtil::GetScreenSize(int* width, int* height) {
#ifdef OS_WIN
  *width = GetSystemMetrics(SM_CXSCREEN);
  *height = GetSystemMetrics(SM_CYSCREEN);
#else
  *width = 1920;
  *height = 1080;
#endif
}

int WinDpiUtil::GetMonitorCount() {
#ifdef OS_WIN
  return GetSystemMetrics(SM_CMONITORS);
#else
  return 1;
#endif
}

bool WinDpiUtil::IsHighDpi() {
  return GetSystemDpi() >= 120;  // 125%+
}

void WinDpiUtil::SetProcessDpiAwareness() {
#ifdef OS_WIN
  // Windows 8.1+
  typedef void(WINAPI* SetProcessDpiAwarenessFunc)(int);
  auto func = reinterpret_cast<SetProcessDpiAwarenessFunc>(
      GetProcAddress(GetModuleHandle(L"shcore.dll"),
                     "SetProcessDpiAwareness"));

  if (func) {
    func(2);  // PROCESS_PER_MONITOR_DPI_AWARE
    return;
  }

  // Windows Vista+
  SetProcessDPIAware();
#endif
}

int WinDpiUtil::CssToPhysical(int css_pixels, int dpi) {
  return static_cast<int>(css_pixels * dpi / 96.0);
}

int WinDpiUtil::PhysicalToCss(int physical_pixels, int dpi) {
  return static_cast<int>(physical_pixels * 96.0 / dpi);
}

// ============================================================
// AdaptiveCSS
// ============================================================

std::string AdaptiveCSS::GenerateCSSVariables(
    const DpiConfig& dpi,
    const FontConfig& font,
    const SpacingConfig& spacing) {
  std::ostringstream css;

  css << ":root {\n";
  css << "  /* DPI Scale */\n";
  css << "  --dpi-scale: " << dpi.scale_factor << ";\n";
  css << "  --dpi: " << dpi.dpi << ";\n\n";

  css << "  /* Fonts */\n";
  css << "  --font-family: " << font.family << ";\n";
  css << "  --font-title: " << font.title_size << "px;\n";
  css << "  --font-body: " << font.body_size << "px;\n";
  css << "  --font-caption: " << font.caption_size << "px;\n";
  css << "  --font-ui: " << font.ui_size << "px;\n\n";

  css << "  /* Spacing */\n";
  css << "  --pad-sm: " << spacing.padding_small << "px;\n";
  css << "  --pad-md: " << spacing.padding_medium << "px;\n";
  css << "  --pad-lg: " << spacing.padding_large << "px;\n";
  css << "  --pad-xl: " << spacing.padding_xlarge << "px;\n\n";

  css << "  /* Layout */\n";
  css << "  --toolbar-height: " << spacing.toolbar_height << "px;\n";
  css << "  --tab-height: " << spacing.tab_height << "px;\n";
  css << "  --sidebar-width: " << spacing.sidebar_width << "px;\n";
  css << "  --statusbar-height: " << spacing.statusbar_height << "px;\n";
  css << "  --margin-window: " << spacing.margin_window << "px;\n";
  css << "  --margin-content: " << spacing.margin_content << "px;\n";
  css << "}\n";

  return css.str();
}

std::string AdaptiveCSS::GenerateMediaQueries() {
  return R"(
    @media (max-width: 1023px) {
      :root {
        --font-title: 10px;
        --font-body: 9px;
        --toolbar-height: 28px;
        --tab-height: 24px;
        --sidebar-width: 36px;
      }
    }

    @media (min-width: 1920px) {
      :root {
        --font-title: 14px;
        --font-body: 13px;
        --toolbar-height: 40px;
        --tab-height: 36px;
      }
    }

    @media (min-width: 2560px) {
      :root {
        --font-title: 16px;
        --font-body: 15px;
        --toolbar-height: 44px;
        --tab-height: 40px;
        --sidebar-width: 56px;
      }
    }
  )";
}

std::string AdaptiveCSS::GenerateTabStyles(const WindowConfig& window,
                                            int current_width) {
  int tab_width = 150;
  int available = current_width - 60;  // sidebar + padding
  int visible = available / tab_width;
  bool overflow = visible < 10;

  std::ostringstream css;
  css << ".tabs-container {\n";
  css << "  display: flex;\n";
  css << "  overflow: " << (overflow ? "hidden" : "visible") << ";\n";
  css << "}\n\n";

  css << ".tab {\n";
  css << "  min-width: " << tab_width << "px;\n";
  css << "  max-width: " << (overflow ? "120" : "200") << "px;\n";
  css << "  flex-shrink: 0;\n";
  css << "}\n";

  return css.str();
}

std::string AdaptiveCSS::GenerateToolbarStyles(const SpacingConfig& spacing) {
  std::ostringstream css;
  css << ".toolbar {\n";
  css << "  height: " << spacing.toolbar_height << "px;\n";
  css << "  padding: 0 " << spacing.padding_medium << "px;\n";
  css << "}\n\n";

  css << ".omnibox {\n";
  css << "  height: " << (spacing.toolbar_height - 8) << "px;\n";
  css << "  font-size: " << spacing.padding_large << "px;\n";
  css << "}\n";

  return css.str();
}

std::string AdaptiveCSS::GetCompactStyles() {
  return R"(
    .sidebar { width: 36px; }
    .toolbar { height: 28px; }
    .tab { height: 24px; font-size: 10px; }
    .statusbar { height: 20px; font-size: 9px; }
    .omnibox { font-size: 10px; padding: 2px 6px; }
  )";
}

std::string AdaptiveCSS::GetUltraHDStyles() {
  return R"(
    .sidebar { width: 56px; }
    .toolbar { height: 44px; }
    .tab { height: 40px; font-size: 14px; }
    .statusbar { height: 32px; font-size: 13px; }
    .omnibox { font-size: 14px; padding: 6px 12px; }
  )";
}

}  // namespace ui
}  // namespace anonymus
