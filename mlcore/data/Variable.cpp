#include "Variable.h"

namespace mlcore
{
  Variable::Variable(const std::string& name, std::size_t num)
  :
    name_(name),
    num_(num)
  {}

  const std::string& Variable::name() const
  {
    return name_;
  }

  std::size_t Variable::num() const
  {
    return num_;
  }
}
