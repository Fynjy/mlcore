/*
 * DataRow.h
 *
 *  Created on: 2016-05-14
 *      Author: Anton Kochnev
 */

#ifndef DATAROW_H_
#define DATAROW_H_

#include <iterator>

#include "cppcore/string/StringView.h"

namespace mlcore
{
namespace data
{
  class DataRow
  {
  public:
    class Iterator
    :
      public std::iterator<std::input_iterator_tag, cppcore::StringView>
    {
    public:
      Iterator& operator++ ();
      Iterator operator++ (int);
      const cppcore::StringView& operator* () const;
      const cppcore::StringView* operator-> () const;
    };

  public:
    DataRow(std::size_t num);
    Iterator begin() const;
    Iterator end() const;
    std::size_t num() const;

  private:
    std::size_t num_;
  };
}
}

#endif /* DATAROW_H_ */
