#include "DataRow.h"

namespace mlcore
{
  DataRow::DataRow()
  {}

  DataRow::DataRow(std::size_t num, std::vector<std::string>&& values)
  :
    num_(num),
    values_(values)
  {}

  std::size_t DataRow::num() const
  {
    return num_;
  }

  const std::vector<std::string>& DataRow::values() const
  {
    return values_;
  }

  std::size_t DataRow::size() const
  {
    return values_.size();
  }

  const std::string& DataRow::get(std::size_t i) const
  {
    return values_[i];
  }
}
