/*
 * DataSource.cpp
 *
 *  Created on: 2016-05-12
 *      Author: Anton Kochnev
 */

#include <sstream>

#include "DataSource.h"

namespace mlcore
{
namespace data
{
  /*
   * DataSource
   */
  DataSource::~DataSource()
  {}

  DataSource::Iterator DataSource::begin()
  {
    return Iterator(this);
  }

  DataSource::Iterator DataSource::end()
  {
    return Iterator(nullptr);
  }

  /*
   * CSVDataSource
   */
  CSVDataSource::CSVDataSource()
  {}

  CSVDataSource::~CSVDataSource()
  {}

  void CSVDataSource::open(const std::string& filename)
  {
    ifs_.open(filename.c_str());

    if (!ifs_)
    {
      std::ostringstream oss;
      oss << "Can't open file: '" << filename << "'";
      throw std::runtime_error(oss.str());
    }
  }

  std::vector<std::string> CSVDataSource::header()
  {
    std::vector<std::string> names;
    std::string line;
    std::getline(ifs_, line);
    std::istringstream iss(line);
    std::string name;

    while (std::getline(iss, name, separator_))
    {
      names.push_back(name);
    }

    return names;
  }
}
}
