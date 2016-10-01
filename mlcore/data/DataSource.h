/*
 * DataSource.h
 *
 *  Created on: 2016-05-12
 *      Author: Anton Kochnev
 */

#ifndef DATASOURCE_H_
#define DATASOURCE_H_

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "cppcore/string/StringView.h"

#include "DataRow.h"

namespace mlcore
{
namespace data
{
  class DataSource
  {
  public:
    class Iterator
    :
      public std::iterator<std::input_iterator_tag, DataRow>
    {
      friend class DataSource;

    public:
      Iterator& operator++ ();
      Iterator operator++ (int);
      const DataRow& operator* () const;
      const DataRow* operator-> () const;

    private:
      DataSource* source_;

    protected:
      Iterator(DataSource* source);
    };

  public:
    virtual ~DataSource();
    virtual std::vector<std::string> header() = 0;
    virtual bool has_next() = 0;
    virtual DataRow next() = 0;

    Iterator begin();
    Iterator end();
  };

  class CSVDataSource
  :
    public DataSource
  {
  public:
    CSVDataSource();
    virtual ~CSVDataSource();
    void open(const std::string& filename);
    virtual std::vector<std::string> header();

  private:
    std::ifstream ifs_;
    char separator_ = ',';
  };
}
}

#endif /* DATASOURCE_H_ */
