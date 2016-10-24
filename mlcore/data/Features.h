#ifndef MLCORE_FEATURES_H_
#define MLCORE_FEATURES_H_

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cppcore/memory/MemoryPoolAllocator.h"

namespace mlcore
{
  enum class FeatureType
  {
    NotOrdered,
    Ordered
  };

  class Feature
  {
  public:
    FeatureType type;
    std::string name;

  public:
    Feature(FeatureType t, const std::string& n);
  };

  typedef std::vector<Feature> Features;

  class double_less
  :
    public std::binary_function<double, double, bool>
  {
  public:
    bool operator() (const double& left, const double& right) const
    {
      return (left - right < -std::numeric_limits<double>::epsilon());
    }
  };

  class FeatureDomain
  {
  public:
    typedef std::pair<const double, std::size_t> Pair;
    typedef cppcore::MemoryPoolAllocatorType Allocator;

  public:
    FeatureDomain(const Allocator& alloc);
    FeatureDomain(const Allocator& alloc, const Feature& feature);
    std::size_t to_mark(double val);
    double to_val(std::size_t mark) const;
    void compile(std::map<std::size_t, std::size_t>& renum_map);
    std::size_t size() const;
    bool has_na() const;
    const Feature& get_feature() const;

    void trace_mapping(std::ostream& os) const;

    void save(std::ostream& os) const;
    void load(std::istream& is);

  private:
    typedef std::map<double, std::size_t, double_less, Allocator> ValToMark;
    typedef std::vector<double> MarkToVal;

  private:
    Feature feature_;
    ValToMark val_to_mark_;
    MarkToVal mark_to_val_;
    bool has_na_flag_ = false;
    std::size_t na_mark_ = 0;
  };

  class Domains
  :
    public std::vector<FeatureDomain>
  {
  public:
    Domains(const FeatureDomain::Allocator& alloc, std::size_t count);
    Domains(const FeatureDomain::Allocator& alloc, const Features& features);
    void trace_mapping(std::ostream& os) const;
  };

  typedef std::shared_ptr<Domains> DomainsPtr;
}

#endif // MLCORE_FEATURES_H_
