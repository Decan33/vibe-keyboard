#include "keyboard/platform/PlatformLibrary.h"

// Compiling these together proves every header below is self-contained
// (includes what it uses).
#include "keyboard/platform/IAlwaysOnTopController.h"
#include "keyboard/platform/IInputInjector.h"
#include "keyboard/platform/IPreferencesStore.h"
#include "keyboard/platform/ISystemMetrics.h"
#include "keyboard/platform/IWindowTransparency.h"
#include "keyboard/platform/SystemMetricsConversion.h"
#include "keyboard/platform/VirtualKeyMapping.h"
#include "keyboard/platform/Win32AlwaysOnTopController.h"
#include "keyboard/platform/Win32InputInjector.h"
#include "keyboard/platform/Win32PreferencesStore.h"
#include "keyboard/platform/Win32SystemMetrics.h"
#include "keyboard/platform/Win32WindowTransparency.h"
