#include "alone/assemble.hh"

#include "alone/edge.hh"
#include "alone/features.hh"
#include "search/applied.hh"

#include <iostream>

namespace alone {

std::ostream &JustText(std::ostream &o, const search::Applied final) {
  const std::vector<const std::string*> &words = static_cast<const Edge*>(final.GetNote().vp)->Words();
  if (words.empty()) return o;
  const search::Applied *child = final.Children();
  std::vector<const std::string*>::const_iterator i(words.begin());
  for (; i != words.end() - 1; ++i) {
    if (*i) {
      o << **i << ' ';
    } else {
      JustText(o, *child) << ' ';
      ++child;
    }
  }

  if (*i) {
    if (**i != "</s>") {
      o << **i;
    }
  } else {
    JustText(o, *child);
  }

  return o;
}

void SumFeatures(const search::Applied final, feature::Adder &to) {
  to.Add(static_cast<const Edge*>(final.GetNote().vp)->Features());
  for (search::Arity i = 0; i < final.GetArity(); ++i) {
    SumFeatures(final.Children()[i], to);
  }
}

std::ostream &SingleLine(std::ostream &o, const search::Applied final, const feature::WeightsBase &weights) {
  if (!final.Valid()) {
    return o << "NO PATH FOUND";
  }
  feature::Adder adder;
  SumFeatures(final, adder);
  feature::Vector vec;
  adder.Finish(vec);
  JustText(o, final);
  o << " ||| ";
  weights.Write(o, vec);
  return o << " ||| " << final.GetScore();
}

namespace {

void MakeIndent(std::ostream &o, const char *indent_str, unsigned int level) {
  for (unsigned int i = 0; i < level; ++i)
    o << indent_str;
}

void DetailedAppliedInternal(std::ostream &o, const search::Applied final, const char *indent_str, unsigned int indent) {
  o << "(\n";
  MakeIndent(o, indent_str, indent);
  const std::vector<const std::string*> &words = static_cast<const Edge*>(final.GetNote().vp)->Words();
  const search::Applied *child = final.Children();
  for (std::vector<const std::string*>::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (*i) {
      o << **i;
      if (i == words.end() - 1) {
        o << '\n';
        MakeIndent(o, indent_str, indent);
      } else {
        o << ' ';
      }
    } else {
      // One extra indent from the line we're currently on.  
      o << indent_str;
      DetailedAppliedInternal(o, *child, indent_str, indent + 1);
      for (unsigned int i = 0; i < indent; ++i) o << indent_str;
      ++child;
    }
  }
  o << ")=" << final.GetScore() << '\n';
}
} // namespace

void DetailedApplied(std::ostream &o, const search::Applied final, const char *indent_str) {
  DetailedAppliedInternal(o, final, indent_str, 0);
}

} // namespace alone
