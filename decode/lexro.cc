#include "decode/lexro.hh"
#include "util/exception.hh"

namespace decode {

void LexicalizedReordering::Init(FeatureInit &feature_init) {
  UTIL_THROW_IF(!feature_init.PhraseAccess().lexical_reordering, util::Exception,
      "requested lexicalized reordering but feature values are missing in phrase access");
  phrase_start_ = util::PODField<std::size_t>(feature_init.HypothesisLayout());
  phrase_access_ = &feature_init.PhraseAccess();
}

void LexicalizedReordering::ScoreHypothesisWithSourcePhrase(
    const Hypothesis &hypothesis, const SourcePhrase source_phrase, ScoreCollector &collector) const {
  // store phrase start index in layout; phrase end is already stored in the hypothesis
  phrase_start_(collector.NewHypothesis()) = source_phrase.Span().first;
  // score with backward lexro
  if (hypothesis.Previous()) {
    std::size_t index = BACKWARD + PhraseRelation(hypothesis, source_phrase.Span());
    collector.AddDense(1, phrase_access_->lexical_reordering(hypothesis.Target())[index]);
  }
}

void LexicalizedReordering::ScoreHypothesisWithPhrasePair(
    const Hypothesis &hypothesis, PhrasePair phrase_pair, ScoreCollector &collector) const {
  std::size_t index = FORWARD + PhraseRelation(hypothesis, phrase_pair.source_phrase.Span());
  float score = phrase_access_->lexical_reordering(&phrase_pair.target_phrase)[index];
  collector.AddDense(0, score);
}

void LexicalizedReordering::ScoreFinalHypothesis(
    const Hypothesis &hypothesis, ScoreCollector &collector) const {
  std::size_t index = BACKWARD + MONOTONE;
  collector.AddDense(1, phrase_access_->lexical_reordering(hypothesis.Target())[index]);
}

LexicalizedReordering::Relation LexicalizedReordering::PhraseRelation(
    const Hypothesis &hypothesis, SourceSpan span) const {
  if (hypothesis.SourceEndIndex() == span.first) {
    return MONOTONE;
  } else if (phrase_start_(&hypothesis) == span.second) {
    return SWAP;
  } else {
    return DISCONTINUOUS;
  }
}

std::string LexicalizedReordering::FeatureDescription(std::size_t index) const {
  assert(index < DenseFeatureCount());
  switch(index) {
    case 0: return "forward lexicalized reordering";
    case 1: return "backward lexicalized reordering";
    default: return "no such feature!";
  }
}

} // namespace decode
