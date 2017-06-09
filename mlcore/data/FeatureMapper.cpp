#include "FeatureMapper.h"

#include <limits>

#include "DataAnalyser.h"

namespace mlcore
{
  /*
   * NumberFeatureMapper
   */
  NumberFeatureMapper::NumberFeatureMapper()
  {}

  NumberFeatureMapper::~NumberFeatureMapper()
  {}

  double NumberFeatureMapper::map(const std::string& str)
  {
    try
    {
      std::string::size_type sz;
      const auto val = std::stod(str, &sz);

      if (sz == str.size())
      {
        return val;
      }
    }
    catch (const std::exception&)
    {}

    return std::numeric_limits<double>::quiet_NaN();
  }

  FeatureType NumberFeatureMapper::feature_type() const
  {
    return FeatureType::Ordered;
  }

  bool NumberFeatureMapper::match(const VariableInfo& info)
  {
    for (const auto& d : info.detectors)
    {
      if (dynamic_cast<const UnclassifiedDetector*>(d.get()))
      {
        if (d->size() > 1)
        {
          return false;
        }
      }
      else if (!dynamic_cast<const NumberTypeDetector*>(d.get()))
      {
        if (d->size() > 0)
        {
          return false;
        }
      }
    }
  }

  /*
   * CategoryFeatureMapper
   */
  CategoryFeatureMapper::~CategoryFeatureMapper()
  {}

  double CategoryFeatureMapper::map(const std::string& str)
  {
    const auto sz = mapping_.size();
    const auto res = mapping_.emplace(str, sz);
    return res.first->second;
  }

  FeatureType CategoryFeatureMapper::feature_type() const
  {
    return FeatureType::NotOrdered;
  }

  bool CategoryFeatureMapper::match(const VariableInfo& info)
  {
    return true;
  }

  /*
   * DateFeatureMapper
   */
  DateFeatureMapper::~DateFeatureMapper()
  {}

  double DateFeatureMapper::map(const std::string& str)
  {
    int day = 0;
    int month = 0;
    int year = 0;
    sscanf_s(str.c_str(), "%4d-%2d-%2d", &month, &day, &year);
    return (year * 10000 + month * 100 + day);
  }

  FeatureType DateFeatureMapper::feature_type() const
  {
    return FeatureType::Ordered;
  }

  bool DateFeatureMapper::match(const VariableInfo& info)
  {
    for (const auto& d : info.detectors)
    {
      if (dynamic_cast<const UnclassifiedDetector*>(d.get()))
      {
        if (d->size() > 1)
        {
          return false;
        }
      }
      else if (!dynamic_cast<const DateFeatureMapper*>(d.get()))
      {
        if (d->size() > 0)
        {
          return false;
        }
      }
    }
  }
}
