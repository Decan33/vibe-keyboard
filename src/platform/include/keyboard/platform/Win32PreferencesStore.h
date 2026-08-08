#pragma once

#include <filesystem>

#include "keyboard/platform/IPreferencesStore.h"

namespace osk::platform {

// Resolves %LOCALAPPDATA%\OnScreenKeyboard via SHGetKnownFolderPath (safer
// than reading the LOCALAPPDATA environment variable directly -- avoids any
// env-var-tampering vector, and is the Microsoft-recommended API for this).
// This is what src/app calls in practice; Win32PreferencesStore's
// constructor takes the directory explicitly so tests can inject a
// throwaway temp directory instead of touching the real one.
std::filesystem::path ResolveDefaultPreferencesDirectory();

// Persists Preferences as a JSON file under the given directory. A
// corrupted, missing, or partially-written file must never prevent the app
// from starting: Load() always returns *something* usable (falling back to
// default-constructed Preferences on any parse/type failure) rather than
// throwing, extending Preferences' own "never leave settings unusable"
// philosophy down to the file layer. Save() writes to a temp file and
// renames it into place, so a crash mid-write can never leave a half-written
// file behind.
class Win32PreferencesStore : public IPreferencesStore {
 public:
  explicit Win32PreferencesStore(std::filesystem::path directory);

  core::Preferences Load() const override;
  void Save(const core::Preferences& preferences) override;

 private:
  std::filesystem::path FilePath() const;

  std::filesystem::path directory_;
};

}  // namespace osk::platform
