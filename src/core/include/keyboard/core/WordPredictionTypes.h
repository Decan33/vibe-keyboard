#pragma once

#include <cstdint>
#include <string>

namespace osk::core {

// A ranked suggestion. score is the raw frequency it was ranked by — higher
// is more likely — not a normalized probability.
struct Suggestion {
  std::string word;
  double score;

  bool operator==(const Suggestion&) const = default;
};

struct WordFrequency {
  std::string word;
  std::uint64_t frequency;

  bool operator==(const WordFrequency&) const = default;
};

// How often `secondWord` was observed immediately following `firstWord`.
struct BigramFrequency {
  std::string firstWord;
  std::string secondWord;
  std::uint64_t frequency;

  bool operator==(const BigramFrequency&) const = default;
};

}  // namespace osk::core
