#include "keyboard/core/NgramWordPredictor.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

class FakeWordPredictionDataSource final : public IWordPredictionDataSource {
 public:
  std::vector<WordFrequency> LoadUnigrams() const override {
    return {
        {.word = "the", .frequency = 100},
        {.word = "cat", .frequency = 50},
        {.word = "dog", .frequency = 45},
        {.word = "car", .frequency = 40},
        {.word = "cats", .frequency = 30},
        {.word = "can", .frequency = 20},
        {.word = "bat", .frequency = 40},
        {.word = "bag", .frequency = 40},
    };
  }

  std::vector<BigramFrequency> LoadBigrams() const override {
    return {
        {.firstWord = "the", .secondWord = "cat", .frequency = 10},
        {.firstWord = "the", .secondWord = "dog", .frequency = 8},
        {.firstWord = "the", .secondWord = "car", .frequency = 5},
        {.firstWord = "i", .secondWord = "can", .frequency = 12},
    };
  }
};

class NgramWordPredictorTest : public ::testing::Test {
 protected:
  FakeWordPredictionDataSource dataSource_;
  NgramWordPredictor predictor_{dataSource_};
};

std::vector<std::string> WordsOf(const std::vector<Suggestion>& suggestions) {
  std::vector<std::string> words;
  words.reserve(suggestions.size());
  for (const Suggestion& suggestion : suggestions) {
    words.push_back(suggestion.word);
  }
  return words;
}

TEST_F(NgramWordPredictorTest, MaxResultsZeroReturnsEmpty) {
  const std::vector<std::string> preceding = {"the"};
  EXPECT_TRUE(predictor_.Suggest(preceding, "ca", 0).empty());
}

TEST_F(NgramWordPredictorTest, NoPrefixNoContextFallsBackToTopUnigramsByFrequency) {
  const auto suggestions = predictor_.Suggest({}, "", 3);
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"the", "cat", "dog"}));
}

TEST_F(NgramWordPredictorTest, NoPrefixWithKnownContextUsesOnlyBigramContinuations) {
  const std::vector<std::string> preceding = {"the"};
  const auto suggestions = predictor_.Suggest(preceding, "", 3);
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"cat", "dog", "car"}));
}

TEST_F(NgramWordPredictorTest, NoPrefixWithUnknownContextFallsBackToTopUnigrams) {
  const std::vector<std::string> preceding = {"unknownword"};
  const auto suggestions = predictor_.Suggest(preceding, "", 2);
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"the", "cat"}));
}

TEST_F(NgramWordPredictorTest, PrefixWithNoContextRanksByRawFrequency) {
  const auto suggestions = predictor_.Suggest({}, "ca", 10);
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"cat", "car", "cats", "can"}));
}

TEST_F(NgramWordPredictorTest, PrefixWithContextBoostsBigramContinuationsAheadOfRawFrequency) {
  const std::vector<std::string> preceding = {"the"};
  const auto suggestions = predictor_.Suggest(preceding, "ca", 10);
  // "cat" and "car" follow "the" in the training data, so they rank ahead
  // of "cats" and "can" even though those have higher raw frequency.
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"cat", "car", "cats", "can"}));
}

TEST_F(NgramWordPredictorTest, MaxResultsTruncatesRankedList) {
  const auto suggestions = predictor_.Suggest({}, "ca", 2);
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"cat", "car"}));
}

TEST_F(NgramWordPredictorTest, PrefixWithNoMatchesReturnsEmpty) {
  const auto suggestions = predictor_.Suggest({}, "xyz", 5);
  EXPECT_TRUE(suggestions.empty());
}

TEST_F(NgramWordPredictorTest, EqualFrequencyTiesBreakAlphabetically) {
  const auto suggestions = predictor_.Suggest({}, "ba", 10);
  EXPECT_EQ(WordsOf(suggestions), (std::vector<std::string>{"bag", "bat"}));
}

TEST_F(NgramWordPredictorTest, ScoreReflectsTheFrequencyItWasRankedBy) {
  const std::vector<std::string> preceding = {"the"};
  const auto suggestions = predictor_.Suggest(preceding, "", 1);
  ASSERT_EQ(suggestions.size(), 1U);
  EXPECT_EQ(suggestions[0].word, "cat");
  EXPECT_DOUBLE_EQ(suggestions[0].score, 10.0);
}

TEST_F(NgramWordPredictorTest, PrefixMatchingIsExactPrefixNotSubstring) {
  // "at" is a substring of "cat"/"bat" but not a prefix of anything in the
  // corpus, so it should match nothing.
  const auto suggestions = predictor_.Suggest({}, "at", 10);
  EXPECT_TRUE(suggestions.empty());
}

}  // namespace
}  // namespace osk::core
