/*
 * VariableType.h
 *
 *  Created on: 2016-05-11
 *      Author: Anton Kochnev
 */

#ifndef VARIABLETYPE_H_
#define VARIABLETYPE_H_

#include <string>

namespace mlcore
{
namespace data
{
  class VariableType
  {
  public:
    VariableType(const std::string& name);
    virtual ~VariableType();
    const std::string& name() const;
    virtual bool match(const std::string& str) const = 0;

  private:
    std::string name_;
  };

  class IntegerVariableType
  :
    public VariableType
  {
  public:
    IntegerVariableType();
    virtual ~IntegerVariableType();
    virtual bool match(const std::string& str) const;
  };
}
}

#endif /* VARIABLETYPE_H_ */
