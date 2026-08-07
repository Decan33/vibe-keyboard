#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "keyboard/core/WordPredictionTypes.h"

namespace osk::core {

// Abstraction over word suggestion, so the statistical n-gram model backing
// it today (NgramWordPredictor) can be replaced by a heavier language model
// later without touching call sites.
class IWordPredictor {
 public:
  virtual ~IWordPredictor() = default;

  // precedingWords: the sentence's already-completed words, most recent
  // last. currentPrefix: the partial word currently being typed, or empty
  // if the user just finished a word (e.g. right after a space) and is
  // starting fresh. Returns at most maxResults suggestions, most likely
  // first.
  virtual std::vector<Suggestion> Suggest(std::span<const std::string> precedingWords,
                                           std::string_view currentPrefix,
                                           std::size_t maxResults) const = 0;
};

}  // namespace osk::core
