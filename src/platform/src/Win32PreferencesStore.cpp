#include "keyboard/platform/Win32PreferencesStore.h"

#include <windows.h>

#include <shlobj.h>
#include <wil/resource.h>

#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <system_error>

namespace osk::platform {

namespace {

using nlohmann::json;

const char* ThemeToString(core::Theme theme) {
  switch (theme) {
    case core::Theme::kSystem: return "system";
    case core::Theme::kLight: return "light";
    case core::Theme::kDark: return "dark";
  }
  return "system";
}

core::Theme ThemeFromString(const std::string& value) {
  if (value == "light") return core::Theme::kLight;
  if (value == "dark") return core::Theme::kDark;
  return core::Theme::kSystem;
}

}  // namespace

std::filesystem::path ResolveDefaultPreferencesDirectory() {
  PWSTR rawPath = nullptr;
  const HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &rawPath);
  const wil::unique_cotaskmem_string path(rawPath);

  if (FAILED(result) || !path) {
    // Best-effort fallback so the app always has *somewhere* writable to
    // persist settings, rather than failing to start.
    return std::filesystem::temp_directory_path() / "OnScreenKeyboard";
  }
  return std::filesystem::path(path.get()) / "OnScreenKeyboard";
}

Win32PreferencesStore::Win32PreferencesStore(std::filesystem::path directory) : directory_(std::move(directory)) {}

std::filesystem::path Win32PreferencesStore::FilePath() const { return directory_ / "preferences.json"; }

core::Preferences Win32PreferencesStore::Load() const {
  core::Preferences preferences;

  std::ifstream file(FilePath(), std::ios::binary);
  if (!file) {
    return preferences;
  }

  // A single catch around the whole parse+apply sequence, deliberately: any
  // failure (malformed JSON, wrong field types, ...) discards partial
  // results and falls back to full defaults, rather than risking a
  // half-applied settings state. Narrow to nlohmann's own exception
  // hierarchy -- not a blanket `catch (...)` -- since that's the only
  // failure mode this code path can actually produce.
  try {
    const json data = json::parse(file);

    preferences.SetTheme(ThemeFromString(data.value("theme", std::string{"system"})));
    preferences.SetTransparencyPercent(data.value("transparencyPercent", preferences.GetTransparencyPercent()));
    preferences.SetWindowScale(data.value("windowScale", preferences.GetWindowScale()));
    preferences.SetKeySize(data.value("keySize", preferences.GetKeySize()));
    preferences.SetActivePanelOrder(data.value("activePanelOrder", std::vector<core::PanelId>{}));
    preferences.SetDwellEnabled(data.value("dwellEnabled", preferences.IsDwellEnabled()));
    preferences.SetDwellDelay(std::chrono::milliseconds{
        data.value("dwellDelayMs", static_cast<std::int64_t>(preferences.GetDwellDelay().count()))});

    if (data.contains("keyRepeatInitialDelayMs") && !data.at("keyRepeatInitialDelayMs").is_null()) {
      preferences.SetKeyRepeatInitialDelay(
          std::chrono::milliseconds{data.at("keyRepeatInitialDelayMs").get<std::int64_t>()});
    }
    if (data.contains("keyRepeatIntervalMs") && !data.at("keyRepeatIntervalMs").is_null()) {
      preferences.SetKeyRepeatInterval(std::chrono::milliseconds{data.at("keyRepeatIntervalMs").get<std::int64_t>()});
    }
  } catch (const json::exception&) {
    return core::Preferences{};
  }

  return preferences;
}

void Win32PreferencesStore::Save(const core::Preferences& preferences) {
  json data;
  data["theme"] = ThemeToString(preferences.GetTheme());
  data["transparencyPercent"] = preferences.GetTransparencyPercent();
  data["windowScale"] = preferences.GetWindowScale();
  data["keySize"] = preferences.GetKeySize();
  data["activePanelOrder"] = preferences.GetActivePanelOrder();
  data["dwellEnabled"] = preferences.IsDwellEnabled();
  data["dwellDelayMs"] = static_cast<std::int64_t>(preferences.GetDwellDelay().count());

  if (const auto initialDelay = preferences.GetKeyRepeatInitialDelay(); initialDelay.has_value()) {
    data["keyRepeatInitialDelayMs"] = static_cast<std::int64_t>(initialDelay->count());
  } else {
    data["keyRepeatInitialDelayMs"] = nullptr;
  }

  if (const auto interval = preferences.GetKeyRepeatInterval(); interval.has_value()) {
    data["keyRepeatIntervalMs"] = static_cast<std::int64_t>(interval->count());
  } else {
    data["keyRepeatIntervalMs"] = nullptr;
  }

  std::error_code ec;
  std::filesystem::create_directories(directory_, ec);
  // ec deliberately ignored here: a real creation failure surfaces next via
  // the ofstream::open failure below, handled uniformly.

  const std::filesystem::path tempPath = std::filesystem::path(FilePath()).concat(".tmp");
  {
    std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
    if (!file) {
      return;  // Best-effort, matching the rest of osk::platform.
    }
    file << data.dump(2);
  }

  // Rename-over-existing is how the write is made crash-safe: a process
  // dying mid-`file <<` above leaves only the .tmp file, never a
  // half-written preferences.json.
  std::filesystem::rename(tempPath, FilePath(), ec);
}

}  // namespace osk::platform
