/*
 * VariableType.cpp
 *
 *  Created on: 2016-05-11
 *      Author: Anton Kochnev
 */

#include <cctype>

#include "VariableType.h"

namespace mlcore
{
namespace data
{
  /*
   * VariableType
   */
  VariableType::VariableType(const std::string& name)
  :
    name_(name)
  {}

  VariableType::~VariableType()
  {}

  const std::string& VariableType::name() const
  {
    return name_;
  }

  /*
   * IntegerVariableType
   */
  IntegerVariableType::IntegerVariableType()
  :
    VariableType("Integer")
  {}

  IntegerVariableType::~IntegerVariableType()
  {}

  bool IntegerVariableType::match(const std::string& str) const
  {
    for (std::size_t i = 0; i < str.length(); ++i)
    {
      if (!std::isdigit(str[i]))
      {
        return false;
      }
    }

    return true;
  }
}
}
