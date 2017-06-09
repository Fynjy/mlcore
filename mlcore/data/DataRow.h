#ifndef DATAROW_H_
#define DATAROW_H_

#include <string>
#include <vector>

namespace mlcore
{
  class DataRow
  {
  public:
    DataRow();
    DataRow(std::size_t num, std::vector<std::string>&& values);
    std::size_t num() const;
    const std::vector<std::string>& values() const;
    std::size_t size() const;
    const std::string& get(std::size_t i) const;
    void clear();

  private:
    std::size_t num_ = 0;
    std::vector<std::string> values_;
  };
}

#endif /* DATAROW_H_ */
