#ifndef VARIABLEDOMAIN_H_
#define VARIABLEDOMAIN_H_

#include <map>
#include <string>

namespace mlcore
{
  class VariableDomain
  {
  public:
    typedef std::map<std::string, std::size_t> Counts;

  public:
    std::size_t size() const;
    void add(const std::string& str, std::size_t c = 1);
    const Counts& counts() const;

  private:
    Counts counts_;
  };
}

#endif /* VARIABLEDOMAIN_H_ */
