/*
 * VariableDomain.h
 *
 *  Created on: 2016-05-09
 *      Author: Anton Kochnev
 */

#ifndef VARIABLEDOMAIN_H_
#define VARIABLEDOMAIN_H_

#include <map>
#include <string>

namespace mlcore
{
namespace data
{
  /**
   * Represents values of the variable in the date.
   */
  class VariableDomain
  {
  public:
    std::size_t count() const;
    void add(const std::string& str);
  };
}
}

#endif /* VARIABLEDOMAIN_H_ */
