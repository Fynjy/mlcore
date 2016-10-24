#include "Features.h"

#include "cppcore/io/Serialization.h"

namespace mlcore
{
  namespace
  {
    const std::size_t NA_MARK = std::numeric_limits<std::size_t>::max();
  }

  /*
   * Feature
   */
  Feature::Feature(FeatureType t, const std::string& n)
  :
    type(t),
    name(n)
  {}

  /*
   * FeatureDomain
   */
  FeatureDomain::FeatureDomain(const Allocator& alloc)
  :
    feature_(FeatureType::NotOrdered, ""),
    val_to_mark_(double_less(), alloc)
  {}

  FeatureDomain::FeatureDomain(const Allocator& alloc, const Feature& feature)
  :
    feature_(feature),
    val_to_mark_(double_less(), alloc)
  {}

  std::size_t FeatureDomain::to_mark(double val)
  {
    if (std::isnan(val))
    {
      has_na_flag_ = true;
      return NA_MARK;
    }

    auto it = val_to_mark_.find(val);

    if (it == val_to_mark_.end())
    {
      const auto sz = val_to_mark_.size();
      it = val_to_mark_.insert(std::make_pair(val, sz)).first;
    }

    return it->second;
  }

  double FeatureDomain::to_val(std::size_t mark) const
  {
    return mark_to_val_[mark];
  }

  void FeatureDomain::compile(std::map<std::size_t, std::size_t>& renum_map)
  {
    for (auto& v : val_to_mark_)
    {
      const auto sz = renum_map.size();
      renum_map[v.second] = sz;
      v.second = sz;
      mark_to_val_.push_back(v.first);
    }

    if (has_na())
    {
      const auto sz = renum_map.size();
      mark_to_val_.push_back(std::numeric_limits<double>::quiet_NaN());
      renum_map[NA_MARK] = sz;
    }

    val_to_mark_.clear();
  }

  std::size_t FeatureDomain::size() const
  {
    return mark_to_val_.size();
  }

  bool FeatureDomain::has_na() const
  {
    return has_na_flag_;
  }

  const Feature& FeatureDomain::get_feature() const
  {
    return feature_;
  }

  void FeatureDomain::trace_mapping(std::ostream& os) const
  {
    for (std::size_t i = 0; i < size(); ++i)
    {
      os << i << ':' << mark_to_val_[i] << ", ";
    }
  }

  void FeatureDomain::save(std::ostream& os) const
  {
    cppcore::write(os, static_cast<int>(feature_.type));
    cppcore::write(os, feature_.name);

    cppcore::write(os, mark_to_val_.size());
    os.write(reinterpret_cast<const char*>(&mark_to_val_[0]), sizeof(double) * mark_to_val_.size());

    cppcore::write(os, has_na_flag_);
    cppcore::write(os, na_mark_);
  }

  void FeatureDomain::load(std::istream& is)
  {
    int type;
    cppcore::read(is, type);
    feature_.type = static_cast<FeatureType>(type);
    cppcore::read(is, feature_.name);

    std::size_t sz = 0;
    cppcore::read(is, sz);
    mark_to_val_.resize(sz);
    is.read(reinterpret_cast<char*>(&mark_to_val_[0]), sizeof(double) * mark_to_val_.size());

    cppcore::read(is, has_na_flag_);
    cppcore::read(is, na_mark_);
  }

  /*
   * Domains
   */
  Domains::Domains(const FeatureDomain::Allocator& alloc, std::size_t count)
  :
    std::vector<FeatureDomain>(count, FeatureDomain(alloc))
  {}

  Domains::Domains(const FeatureDomain::Allocator& alloc, const Features& features)
  {
    for (const auto& f : features)
    {
      emplace_back(alloc, f);
    }
  }

  void Domains::trace_mapping(std::ostream& os) const
  {
    for (std::size_t n = 0; n < size(); ++n)
    {
      os << "FEATURE #" << n << std::endl;
      (*this)[n].trace_mapping(os);
      os << std::endl;
    }
  }
}
