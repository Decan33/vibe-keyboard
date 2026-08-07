#pragma once

#include <vector>

#include "keyboard/core/WordPredictionTypes.h"

namespace osk::core {

// Supplies the raw frequency data NgramWordPredictor ranks suggestions
// from. Kept separate from the predictor itself so tests can supply a tiny
// synthetic model instead of a real shipped dictionary, and so the real
// data (sourcing/licensing still a follow-up) can be swapped in later
// without touching prediction logic.
class IWordPredictionDataSource {
 public:
  virtual ~IWordPredictionDataSource() = default;

  virtual std::vector<WordFrequency> LoadUnigrams() const = 0;
  virtual std::vector<BigramFrequency> LoadBigrams() const = 0;
};

}  // namespace osk::core
