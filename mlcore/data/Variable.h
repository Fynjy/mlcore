#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <string>

namespace mlcore
{
  class Variable
  {
  public:
    Variable(const std::string& name, std::size_t num);
    const std::string& name() const;
    std::size_t num() const;

  private:
    std::string name_;
    std::size_t num_ = 0;
  };
}

#endif /* VARIABLE_H_ */
