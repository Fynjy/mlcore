#ifndef DATASOURCE_H_
#define DATASOURCE_H_

#include <fstream>
#include <string>
#include <vector>

#include "cppcore/string/StringView.h"

#include "DataRow.h"

namespace mlcore
{
  class DataSource
  {
  public:
    virtual ~DataSource()
    {}

    virtual const std::vector<std::string>& header() = 0;
    virtual bool has_next() = 0;
    virtual const DataRow& next() = 0;
  };

  class CSVDataSource
  :
    public DataSource
  {
  public:
    CSVDataSource();
    virtual ~CSVDataSource();
    void open(const std::string& filename);
    virtual const std::vector<std::string>& header() override;
    virtual bool has_next() override;
    virtual const DataRow& next() override;

  private:
    std::ifstream ifs_;
    bool has_data_ = true;
    std::vector<std::string> header_;
    DataRow current_;

    char separator_ = ',';
    bool has_header_flag_ = true;

  private:
    void read_next();
  };
}

#endif /* DATASOURCE_H_ */
