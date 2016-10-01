#ifndef MLCORE_FEATURES_H_
#define MLCORE_FEATURES_H_

#include <string>
#include <vector>

#include "cppcore/memory/MemoryPool.h"

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

  class FeatureDomain
  {
  public:
    typedef std::pair<const double, std::size_t> Pair;

    typedef common::memory::StaticPoolAllocator<
      Pair, common::memory::DebugMemoryPool> Allocator;

  public:
    FeatureDomain(const Allocator& alloc);
    FeatureDomain(const Allocator& alloc, const Feature& _feature);
    MarkType to_mark(double val);
    double to_val(MarkType mark) const;
    void compile(std::map<std::size_t, std::size_t>& renum_map);
    std::size_t size() const;
    bool has_na() const;
    void trace_mapping(std::ostream& os) const;

    const Feature& get_feature() const
    {
      return feature;
    }

    void save(std::ostream& os) const;
    void load(std::istream& is);

  private:
    typedef std::map<double, MarkType, double_less, Allocator> ValToMark;
    typedef std::vector<double> MarkToVal;

  private:
    Feature feature;
    ValToMark val_to_mark;
    MarkToVal mark_to_val;
    bool has_na_flag = false;
    MarkType na_mark = 0;
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
