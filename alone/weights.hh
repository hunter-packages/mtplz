// For now, the individual features are not kept.  
#ifndef ALONE_WEIGHTS__
#define ALONE_WEIGHTS__

#include "search/config.hh"
#include "search/types.hh"
#include "util/exception.hh"
#include "util/string_piece.hh"

#include <boost/unordered_map.hpp>

#include <string>

namespace alone {

class WeightParseException : public util::Exception {
  public:
    WeightParseException() {}
    ~WeightParseException() throw() {}
};

class Weights : public search::Weights {
  private:
    typedef search::Weights P;
  public:
    // Parses weights, sets lm_weight_, removes it from map_.
    explicit Weights(StringPiece text);

    search::Score DotNoLM(StringPiece text) const;

    search::Score WordPenalty() const { return word_penalty_; }

    // Mostly for testing.  
    const boost::unordered_map<std::string, search::Score> &GetMap() const { return map_; }

  private:
    float Steal(const std::string &str);

    typedef boost::unordered_map<std::string, search::Score> Map;

    Map map_;

    search::Score word_penalty_;
};

} // namespace alone

#endif // ALONE_WEIGHTS__
