#include "VariableDomain.h"

namespace mlcore
{
  std::size_t VariableDomain::size() const
  {
    return counts_.size();
  }

  void VariableDomain::add(const std::string& str, std::size_t c)
  {
    auto it = counts_.emplace(str, 0);
    it.first->second += c;
  }

  const VariableDomain::Counts& VariableDomain::counts() const
  {
    return counts_;
  }
}
