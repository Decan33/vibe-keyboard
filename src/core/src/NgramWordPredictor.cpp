#include "keyboard/core/NgramWordPredictor.h"

#include <algorithm>

namespace osk::core {

namespace {

bool ByFrequencyDescThenAlpha(const WordFrequency& lhs, const WordFrequency& rhs) {
  if (lhs.frequency != rhs.frequency) {
    return lhs.frequency > rhs.frequency;
  }
  return lhs.word < rhs.word;
}

std::vector<Suggestion> ToSuggestions(const std::vector<WordFrequency>& items, std::size_t maxResults) {
  std::vector<Suggestion> result;
  const std::size_t count = std::min(items.size(), maxResults);
  result.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(Suggestion{.word = items[i].word, .score = static_cast<double>(items[i].frequency)});
  }
  return result;
}

}  // namespace

NgramWordPredictor::NgramWordPredictor(const IWordPredictionDataSource& dataSource) {
  unigramsSortedByWord_ = dataSource.LoadUnigrams();
  std::sort(unigramsSortedByWord_.begin(), unigramsSortedByWord_.end(),
            [](const WordFrequency& lhs, const WordFrequency& rhs) { return lhs.word < rhs.word; });

  unigramsSortedByFrequency_ = unigramsSortedByWord_;
  std::sort(unigramsSortedByFrequency_.begin(), unigramsSortedByFrequency_.end(), ByFrequencyDescThenAlpha);

  for (const BigramFrequency& bigram : dataSource.LoadBigrams()) {
    bigramFollowing_[bigram.firstWord].push_back(
        WordFrequency{.word = bigram.secondWord, .frequency = bigram.frequency});
  }
  for (auto& [word, following] : bigramFollowing_) {
    std::sort(following.begin(), following.end(), ByFrequencyDescThenAlpha);
  }
}

std::vector<WordFrequency> NgramWordPredictor::FindByPrefix(std::string_view prefix) const {
  const auto begin = std::lower_bound(
      unigramsSortedByWord_.begin(), unigramsSortedByWord_.end(), prefix,
      [](const WordFrequency& wf, std::string_view p) { return wf.word < p; });

  std::vector<WordFrequency> matches;
  for (auto it = begin; it != unigramsSortedByWord_.end() && it->word.starts_with(prefix); ++it) {
    matches.push_back(*it);
  }
  return matches;
}

std::vector<Suggestion> NgramWordPredictor::Suggest(std::span<const std::string> precedingWords,
                                                     std::string_view currentPrefix,
                                                     std::size_t maxResults) const {
  if (maxResults == 0) {
    return {};
  }

  const std::vector<WordFrequency>* bigramCandidates = nullptr;
  if (!precedingWords.empty()) {
    const auto it = bigramFollowing_.find(precedingWords.back());
    if (it != bigramFollowing_.end()) {
      bigramCandidates = &it->second;
    }
  }

  if (currentPrefix.empty()) {
    // Next-word mode: only offer plausible continuations of the preceding
    // word, if we have any — falling back to raw frequency would ignore
    // context entirely and defeat the point of "contextual" suggestions.
    if (bigramCandidates != nullptr && !bigramCandidates->empty()) {
      return ToSuggestions(*bigramCandidates, maxResults);
    }
    return ToSuggestions(unigramsSortedByFrequency_, maxResults);
  }

  // Word-completion mode: prefix is the primary filter; preceding-word
  // context boosts matching candidates ahead of same-prefix ones with no
  // contextual support.
  std::unordered_map<std::string, std::uint64_t> bigramFreqByWord;
  if (bigramCandidates != nullptr) {
    bigramFreqByWord.reserve(bigramCandidates->size());
    for (const WordFrequency& candidate : *bigramCandidates) {
      bigramFreqByWord.emplace(candidate.word, candidate.frequency);
    }
  }

  std::vector<WordFrequency> boosted;
  std::vector<WordFrequency> plain;
  for (const WordFrequency& candidate : FindByPrefix(currentPrefix)) {
    const auto found = bigramFreqByWord.find(candidate.word);
    if (found != bigramFreqByWord.end()) {
      boosted.push_back(WordFrequency{.word = candidate.word, .frequency = found->second});
    } else {
      plain.push_back(candidate);
    }
  }
  std::sort(boosted.begin(), boosted.end(), ByFrequencyDescThenAlpha);
  std::sort(plain.begin(), plain.end(), ByFrequencyDescThenAlpha);

  std::vector<WordFrequency> combined = std::move(boosted);
  combined.insert(combined.end(), plain.begin(), plain.end());

  return ToSuggestions(combined, maxResults);
}

}  // namespace osk::core
