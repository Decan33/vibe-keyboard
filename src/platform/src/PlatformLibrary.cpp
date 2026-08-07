#include "keyboard/platform/PlatformLibrary.h"

// Compiling these together proves every interface header below is
// self-contained (includes what it uses). No implementation logic lives
// here — these are pure interfaces; real Win32/WinRT bodies are deferred to
// Windows-side work, per CLAUDE.md.
#include "keyboard/platform/IAlwaysOnTopController.h"
#include "keyboard/platform/IInputInjector.h"
#include "keyboard/platform/IPreferencesStore.h"
#include "keyboard/platform/ISystemMetrics.h"
#include "keyboard/platform/IWindowTransparency.h"
