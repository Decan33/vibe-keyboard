#pragma once

#include "MainWindow.g.h"

#include <memory>
#include <utility>
#include <vector>

#include "keyboard/core/BuiltInPanels.h"
#include "keyboard/core/Dispatcher.h"
#include "keyboard/core/Key.h"
#include "keyboard/core/KeyboardLayout.h"
#include "keyboard/core/ModifierState.h"
#include "keyboard/platform/Win32AlwaysOnTopController.h"
#include "keyboard/platform/Win32InputInjector.h"

namespace winrt::OnScreenKeyboardApp::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

    private:
        void BuildKeyboardUi();
        void OnKeyActivated(const osk::core::Key& key, osk::core::ActivationOverride activationOverride);

        // Reflects modifierState_'s latched/CapsLock state back onto every
        // modifier-toggle button's visuals (e.g. Shift highlighted while
        // latched) -- called after every activation, since any key
        // activation can change modifier state (toggling one, or consuming
        // latches via a non-modifier keypress).
        void UpdateModifierVisuals();

        osk::core::ModifierState modifierState_;
        osk::core::Dispatcher dispatcher_{ modifierState_ };
        osk::core::KeyboardLayout layout_{ osk::core::panels::Qwerty() };
        osk::platform::Win32InputInjector injector_;

        // One entry per ToggleModifier/ToggleCapsLock key, populated while
        // building the grid -- lets UpdateModifierVisuals() update each
        // button's highlight without rebuilding or re-walking the layout.
        std::vector<std::pair<osk::core::Key, winrt::Microsoft::UI::Xaml::Controls::Button>> modifierButtons_;

        // Constructed once the real HWND is available, part-way through the
        // constructor -- not default-constructible (no default HWND to be
        // always-on-top *of*).
        std::unique_ptr<osk::platform::Win32AlwaysOnTopController> alwaysOnTop_;
    };
}

namespace winrt::OnScreenKeyboardApp::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
