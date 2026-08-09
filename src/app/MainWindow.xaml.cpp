#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <cstddef>
#include <cstdint>
#include <variant>

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Media;

namespace
{
    // Relative column widths so a row fills the window edge-to-edge with
    // proportionally-sized keys -- e.g. Space genuinely wide and easy to
    // click -- rather than every key getting an identical fixed size
    // regardless of what it represents (the "packed like Windows' own OSK"
    // feedback). Keyed off the stable `Key::id` strings from BuiltInPanels.
    double KeyWidthWeight(const osk::core::Key& key)
    {
        if (key.id == "qwerty.space") return 6.0;
        if (key.id == "qwerty.backspace" || key.id == "qwerty.enter") return 2.0;
        if (key.id == "qwerty.shift_left" || key.id == "qwerty.shift_right") return 2.25;
        if (key.id == "qwerty.tab" || key.id == "qwerty.capslock") return 1.75;
        if (key.id == "qwerty.backslash" || key.id == "qwerty.shift_enter" ||
            key.id == "qwerty.copy" || key.id == "qwerty.paste") return 1.5;
        if (key.id == "qwerty.ctrl_left" || key.id == "qwerty.ctrl_right" ||
            key.id == "qwerty.alt_left" || key.id == "qwerty.alt_right") return 1.25;
        return 1.0;
    }

    bool IsModifierToggleKey(const osk::core::Key& key)
    {
        return std::holds_alternative<osk::core::ToggleModifier>(key.action) ||
               std::holds_alternative<osk::core::ToggleCapsLock>(key.action);
    }
}

namespace winrt::OnScreenKeyboardApp::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        HWND hwnd{};
        auto windowNative = this->try_as<::IWindowNative>();
        winrt::check_bool(bool{ windowNative });
        winrt::check_hresult(windowNative->get_WindowHandle(&hwnd));

        // Priority requirement: stays on top of other apps/windows. On by
        // default since there's no settings UI yet to toggle it (see
        // CLAUDE.md for what's deferred from this v1 slice).
        alwaysOnTop_ = std::make_unique<osk::platform::Win32AlwaysOnTopController>(hwnd);
        alwaysOnTop_->Start();

        // "Never steal focus from the app being typed into" requirement.
        // Verified 2026-08-08 against a real WinUI3 build: button clicks
        // (including via UI Automation's InvokePattern) still register
        // correctly with this flag set -- resolves the risk flagged in
        // CLAUDE.md since 2026-08-07 ("WinUI 3's internal pointer-routing
        // behavior under WS_EX_NOACTIVATE hasn't been prototyped yet").
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, GetWindowLongPtrW(hwnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

        // Native Windows 11 look (Mica) rather than a flat/generic
        // background -- the styling gap raised alongside the layout one.
        this->SystemBackdrop(MicaBackdrop());

        BuildKeyboardUi();
    }

    void MainWindow::BuildKeyboardUi()
    {
        // A Grid of Grids, not nested StackPanels: rows/columns are
        // star-sized so the whole layout stretches to fill the window
        // (packed like Windows' own OSK) and each key's column gets a
        // weight proportional to its real-world importance (e.g. Space).
        Grid rowsGrid;
        rowsGrid.Margin(Thickness{ 6, 6, 6, 6 });
        rowsGrid.RowSpacing(4);

        const auto& rows = layout_.BasePanel().rows;
        for (std::size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
        {
            RowDefinition rowDef;
            rowDef.Height(GridLength{ 1, GridUnitType::Star });
            rowsGrid.RowDefinitions().Append(rowDef);
        }

        for (std::size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
        {
            const osk::core::PanelRow& row = rows[rowIndex];

            Grid rowGrid;
            rowGrid.ColumnSpacing(4);
            Grid::SetRow(rowGrid, static_cast<std::int32_t>(rowIndex));

            for (const osk::core::Key& key : row.keys)
            {
                ColumnDefinition columnDef;
                columnDef.Width(GridLength{ KeyWidthWeight(key), GridUnitType::Star });
                rowGrid.ColumnDefinitions().Append(columnDef);
            }

            for (std::size_t columnIndex = 0; columnIndex < row.keys.size(); ++columnIndex)
            {
                const osk::core::Key& key = row.keys[columnIndex];

                Button button;
                button.Content(box_value(to_hstring(key.label)));
                button.HorizontalAlignment(HorizontalAlignment::Stretch);
                button.VerticalAlignment(VerticalAlignment::Stretch);
                Grid::SetColumn(button, static_cast<std::int32_t>(columnIndex));

                // Captured by value (a copy of `key`), not by reference to
                // the loop variable -- each lambda owns its own Key, so
                // there's no dangling-reference risk once this loop ends.
                button.Click([this, key](IInspectable const&, RoutedEventArgs const&)
                {
                    OnKeyActivated(key, osk::core::ActivationOverride::kNone);
                });
                // "Important" requirement: right-click capitalizes a letter
                // one-off, without touching the sticky Shift latch.
                button.RightTapped([this, key](IInspectable const&, RightTappedRoutedEventArgs const& e)
                {
                    e.Handled(true);
                    OnKeyActivated(key, osk::core::ActivationOverride::kForceShiftOnce);
                });

                if (IsModifierToggleKey(key))
                {
                    modifierButtons_.emplace_back(key, button);
                }

                rowGrid.Children().Append(button);
            }

            rowsGrid.Children().Append(rowGrid);
        }

        RootGrid().Children().Append(rowsGrid);
        UpdateModifierVisuals();
    }

    void MainWindow::OnKeyActivated(const osk::core::Key& key, osk::core::ActivationOverride activationOverride)
    {
        if (std::optional<osk::core::KeyEvent> event = dispatcher_.ActivateKey(key, activationOverride);
            event.has_value())
        {
            injector_.InjectKeyEvent(*event);
        }
        // ToggleModifier/ToggleCapsLock activations return nullopt (they
        // only mutate modifierState_) -- nothing to inject, by design. Every
        // activation can still change modifier state though (toggling one,
        // or consuming latches via a non-modifier keypress), so refresh
        // unconditionally rather than only on the nullopt branch.
        UpdateModifierVisuals();
    }

    void MainWindow::UpdateModifierVisuals()
    {
        for (auto& [key, button] : modifierButtons_)
        {
            bool active = false;
            if (const auto* toggle = std::get_if<osk::core::ToggleModifier>(&key.action))
            {
                active = modifierState_.IsLatched(toggle->modifier);
            }
            else if (std::holds_alternative<osk::core::ToggleCapsLock>(key.action))
            {
                active = modifierState_.IsCapsLockOn();
            }

            if (active)
            {
                button.Background(SolidColorBrush(Windows::UI::Colors::DodgerBlue()));
            }
            else
            {
                button.ClearValue(Control::BackgroundProperty());
            }
        }
    }
}
