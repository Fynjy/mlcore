#include "Filter.h"

namespace mlcore
{
  bool Filter::match(const std::vector<double>& x) const
  {
    return match(x[feature_num]);
  }

  bool Filter::match(double x) const
  {
    if (std::isnan(x))
    {
      return std::isnan(split_point);
    }

    return
      (feature_type == FeatureType::NotOrdered ?
        (x == split_point) :
        (x <= split_point));
  }

  bool Filter::match(std::size_t f_num, double x) const
  {
    if (feature_num != f_num)
    {
      return true;
    }

    return match(x);
  }

  void Filter::save(std::ostream& os) const
  {
    os << feature_num << ' ' << static_cast<int>(feature_type) << ' '
      << split_point << ' ' << std::endl;
  }

  void Filter::load(std::istream& is)
  {
    int t;
    is >> feature_num >> t >> split_point;
    feature_type = static_cast<FeatureType>(t);
  }

  std::ostream& operator<< (std::ostream& os, const Filter& filter)
  {
    os << "x[" << filter.feature_num << "] ";

    if (std::isnan(filter.split_point))
    {
      os << "is NA";
    }
    else
    {
      os << (filter.feature_type == FeatureType::NotOrdered ? "=" : "<=")
        << ' ' << filter.split_point;
    }

    return os;
  }
}
