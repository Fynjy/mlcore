#include <sstream>

#include "DataSource.h"

namespace mlcore
{
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

    if (has_header_flag_ && has_data_)
    {
      read_next();
      header_ = current_.values();
      current_ = DataRow();
    }
  }

  const std::vector<std::string>& CSVDataSource::header()
  {
    return header_;
  }

  bool CSVDataSource::has_next()
  {
    return has_data_;
  }

  const DataRow& CSVDataSource::next()
  {
    read_next();
    return current_;
  }

  void CSVDataSource::read_next()
  {
    std::string line;
    has_data_ = static_cast<bool>(std::getline(ifs_, line));
    has_data_ &= !line.empty();

    if (has_data_)
    {
      std::istringstream iss(line);
      std::string val;
      std::vector<std::string> vals;

      while (std::getline(iss, val, separator_))
      {
        vals.push_back(val);
      }

      current_ = DataRow(current_.num() + 1, std::move(vals));
    }
  }
}
