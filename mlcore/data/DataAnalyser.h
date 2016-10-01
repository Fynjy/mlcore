/*
 * DataAnalyser.h
 *
 *  Created on: 2016-05-12
 *      Author: Anton Kochnev
 */

#ifndef DATAANALYSER_H_
#define DATAANALYSER_H_

#include <vector>

#include "DataSource.h"
#include "Variable.h"
#include "VariableDomain.h"

namespace mlcore
{
namespace data
{
  class DataAnalyser
  {
  public:
    struct VariableInfo
    {
      Variable variable;
      VariableDomain domain;

      VariableInfo(const std::string& name, std::size_t num)
      :
        variable(name, num)
      {}
    };

    typedef std::vector<VariableInfo> VariableInfos;

  public:
    void analyse(DataSource& data_source);
    const VariableInfos& variable_infos() const;

  private:
    VariableInfos info_;
  };
}
}

#endif /* DATAANALYSER_H_ */
