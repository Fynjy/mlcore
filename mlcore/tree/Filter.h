#ifndef MLCORE_TREE_FILTER_H_
#define MLCORE_TREE_FILTER_H_

#include <istream>
#include <ostream>
#include <vector>

#include "mlcore/data/Features.h"

namespace mlcore
{
  class Filter
  {
  public:
    bool match(const std::vector<double>& x) const;
    bool match(double x) const;
    bool match(std::size_t f_num, double x) const;

    void save(std::ostream& os) const;
    void load(std::istream& is);

    friend std::ostream& operator<< (
      std::ostream& os, const Filter& filter);

  public:
    std::size_t feature_num = 0;
    FeatureType feature_type;
    double split_point = 0;
  };
}

#endif // MLCORE_TREE_FILTER_H_
