#ifndef GROUP_DATA_HISTORGAM_H_
#define GROUP_DATA_HISTORGAM_H_

#include <cstddef>
#include <istream>
#include <ostream>

namespace mlcore
{
  class GroupDataHistogram
  {
  public:
    GroupDataHistogram();
    GroupDataHistogram(const Features& features, std::size_t common_length);
    void compile();
    void move(GroupDataHistogram& data_histogram, std::size_t b, std::size_t e);
    void save(std::ostream& os) const;
    void load(std::istream& is);

  private:
  };
}

#endif // GROUP_DATA_HISTORGAM_H_
