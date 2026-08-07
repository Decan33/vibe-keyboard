#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "keyboard/core/IWordPredictionDataSource.h"
#include "keyboard/core/IWordPredictor.h"

namespace osk::core {

// Statistical word prediction: a word-frequency table for completing the
// word currently being typed, plus a bigram (previous-word -> next-word)
// frequency table for ranking suggestions by what commonly follows the
// preceding word. Fully offline, no external dependency.
//
// When completing a prefix, candidates that are also plausible bigram
// continuations of the preceding word are ranked ahead of same-prefix
// candidates that aren't — the closest this model gets to "logical for the
// sentence" without a full language model. Ties break alphabetically for
// determinism.
class NgramWordPredictor final : public IWordPredictor {
 public:
  explicit NgramWordPredictor(const IWordPredictionDataSource& dataSource);

  std::vector<Suggestion> Suggest(std::span<const std::string> precedingWords,
                                   std::string_view currentPrefix,
                                   std::size_t maxResults) const override;

 private:
  std::vector<WordFrequency> FindByPrefix(std::string_view prefix) const;

  std::vector<WordFrequency> unigramsSortedByWord_;
  std::vector<WordFrequency> unigramsSortedByFrequency_;
  std::unordered_map<std::string, std::vector<WordFrequency>> bigramFollowing_;
};

}  // namespace osk::core
