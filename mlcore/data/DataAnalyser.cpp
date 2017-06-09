#include "DataAnalyser.h"

#include <regex>
#include <stdexcept>
#include <sstream>

namespace mlcore
{
  /*
   * UnclassifiedDetector
   */
  UnclassifiedDetector::~UnclassifiedDetector()
  {}

  bool UnclassifiedDetector::match(const std::string& str)
  {
    return true;
  }

  /*
   * NumberTypeDetector
   */
  NumberTypeDetector::~NumberTypeDetector()
  {}

  bool NumberTypeDetector::match(const std::string& str)
  {
    try
    {
      std::string::size_type sz;
      std::stod(str, &sz);
      return (sz == str.size());
    }
    catch (const std::exception&)
    {}

    return false;
  }

  /*
   * DateTypeDetector
   */
  DateTypeDetector::~DateTypeDetector()
  {}

  namespace
  {
    const std::regex date_regex("\\d\\d\\d\\d-\\d\\d-\\d\\d");
  }

  bool DateTypeDetector::match(const std::string& str)
  {
    return std::regex_match(str, date_regex);
  }

  /*
   * VariableInfo
   */
  VariableInfo::VariableInfo(const std::string& name, std::size_t num)
  :
    variable(name, num)
  {}

  std::size_t VariableInfo::size() const
  {
    std::size_t sz = 0;

    for (const auto& d : detectors)
    {
      sz += d->size();
    }

    return sz;
  }

  const VariableDomain& VariableInfo::unclassified() const
  {
    return *detectors.back();
  }

  /*
   * DataAnalyser
   */
  DataAnalyser::DataAnalyser()
  {}

  void DataAnalyser::analyse(DataSource& data_source)
  {
    const auto names = data_source.header();

    for (std::size_t i = 0; i < names.size(); ++i)
    {
      info_.emplace_back(names[i], i);

      auto& detectors = info_.back().detectors;
      detectors.emplace_back(std::make_shared<NumberTypeDetector>());
      detectors.emplace_back(std::make_shared<DateTypeDetector>());
      detectors.emplace_back(std::make_shared<UnclassifiedDetector>());
    }

    while (data_source.has_next())
    {
      const auto& row = data_source.next();
      const auto& values = row.values();

      for (std::size_t i = 0; i < values.size(); ++i)
      {
        auto& info = info_[i];

        for (const auto& detector : info.detectors)
        {
          if (detector->match(values[i]))
          {
            detector->add(values[i]);
            break;
          }
        }
      }
    }
  }

  const VariableInfos& DataAnalyser::variable_infos() const
  {
    return info_;
  }
}
