/*
 * DataAnalyser.cpp
 *
 *  Created on: 2016-05-12
 *      Author: Anton Kochnev
 */

#include "DataAnalyser.h"

namespace mlcore
{
namespace data
{
  void DataAnalyser::analyse(DataSource& data_source)
  {
    const auto names = data_source.header();

    for (std::size_t i = 0; i < names.size(); ++i)
    {
      info_.emplace_back(names[i], i);
    }
  }

  const DataAnalyser::VariableInfos& DataAnalyser::variable_infos() const
  {
    return info_;
  }
}
}
