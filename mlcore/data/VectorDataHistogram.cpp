#include <sstream>
#include <stdexcept>

#include "cppcore/memory/MemoryPoolAllocator.h"
#include "cppcore/io/Serialization.h"

#include "VectorDataHistogram.h"

namespace mlcore
{
  /*
   * VectorDataPoint
   */
  VectorDataPoint::VectorDataPoint(std::size_t* x, double* d)
  :
    x_(x),
    d_(d)
  {}

  std::size_t VectorDataPoint::x(std::size_t feature_num) const
  {
    return x_[feature_num];
  }

  std::size_t& VectorDataPoint::x(std::size_t feature_num)
  {
    return x_[feature_num];
  }

  double VectorDataPoint::d(std::size_t point_num) const
  {
    return d_[point_num];
  }

  double& VectorDataPoint::d(std::size_t point_num)
  {
    return d_[point_num];
  }

  /*
   * VectorDataHistogram
   */
  VectorDataHistogram::VectorDataHistogram()
  :
    memory_pool_(1024 * 64)
  {}

  VectorDataHistogram::VectorDataHistogram(const Features& features, std::size_t points_count)
  :
    points_count_(points_count),
    memory_pool_(1024 * 64),
    domains_(std::make_shared<Domains>(cppcore::MemoryPoolAllocatorType(memory_pool_), features))
  {}

  void VectorDataHistogram::compile()
  {
    if (is_compiled_)
    {
      return;
    }

    typedef std::map<std::size_t, std::size_t> RenumMap;
    typedef std::vector<RenumMap> RenumMaps;
    RenumMaps renum_maps(domains_->size());

    for (std::size_t i = 0; i < domains_->size(); ++i)
    {
      auto& domain = (*domains_)[i];
      domain.compile(renum_maps[i]);
    }

    for (std::size_t j = 0; j < points_.size(); ++j)
    {
      VectorDataPoint& point = points_.back();

      for (std::size_t i = 0; i < features_count(); ++i)
      {
        point.x(i) = renum_maps[i][point.x(i)];
      }
    }

    is_compiled_ = true;
  }

  void VectorDataHistogram::move(VectorDataHistogram& data_histogram, std::size_t b, std::size_t e)
  {
  }

  void VectorDataHistogram::save(std::ostream& os) const
  {
    cppcore::write(os, features_count());
    cppcore::write(os, points_count_);

    for (const auto& domain : *domains_)
    {
      domain.save(os);
    }

    cppcore::write(os, points_.size());

    for (std::size_t i = 0; i < points_.size(); ++i)
    {
      os.write(reinterpret_cast<const char*>(points_[i].x_), features_count() * sizeof(std::size_t));
      os.write(reinterpret_cast<const char*>(points_[i].d_), points_count() * sizeof(double));
    }
  }

  void VectorDataHistogram::push_back()
  {
    cppcore::MemoryPoolAllocator<char> alloc(memory_pool_);
    char* x = alloc.allocate(features_count() * sizeof(std::size_t));
    char* d = alloc.allocate(points_count() * sizeof(double));
    points_.emplace_back(reinterpret_cast<std::size_t*>(x), reinterpret_cast<double*>(d));
  }

  void VectorDataHistogram::load(std::istream& is)
  {
    std::size_t sz;
    cppcore::read(is, sz);
    cppcore::read(is, points_count_);
    domains_.reset(new Domains(cppcore::MemoryPoolAllocatorType(memory_pool_), sz));

    for (auto& domain : *domains_)
    {
      domain.load(is);
    }

    cppcore::read(is, sz);
    cppcore::MemoryPoolAllocator<char> alloc(memory_pool_);

    for (std::size_t i = 0; i < sz; ++i)
    {
      push_back();
      is.read(reinterpret_cast<char*>(points_.back().x_), features_count() * sizeof(std::size_t));
      is.read(reinterpret_cast<char*>(points_.back().d_), points_count() * sizeof(double));
    }
  }

  std::size_t VectorDataHistogram::size() const
  {
    return points_.size();
  }

  std::size_t VectorDataHistogram::features_count() const
  {
    return domains_->size();
  }

  std::size_t VectorDataHistogram::points_count() const
  {
    return points_count_;
  }

  const DomainsPtr& VectorDataHistogram::domains() const
  {
    return domains_;
  }

  bool VectorDataHistogram::is_compiled() const
  {
    return is_compiled_;
  }

  const VectorDataPoint& VectorDataHistogram::operator[] (std::size_t i) const
  {
    return points_[i];
  }

  VectorDataPoint& VectorDataHistogram::operator[] (std::size_t i)
  {
    return points_[i];
  }

  double VectorDataHistogram::mark_to_val(std::size_t feature_num, std::size_t mark) const
  {
    return (*domains_)[feature_num].to_val(mark);
  }

  std::size_t VectorDataHistogram::val_to_mark(std::size_t feature_num, double val)
  {
    return (*domains_)[feature_num].to_mark(val);
  }

  VectorDataPoint& VectorDataHistogram::add(const std::vector<double>& x)
  {
    push_back();

    for (std::size_t i = 0; i < features_count(); ++i)
    {
      points_.back().x(i) = val_to_mark(i, x[i]);
    }

    return points_.back();
  }

  void VectorDataHistogram::trace_memory_stats(std::ostream& os)
  {
    const auto debug_info = memory_pool_.get_debug_info();
    os << "Memory pool stats:\n"
      << "allocation_count = " << debug_info.allocation_count << "\n"
      << "allocated_size = " << debug_info.allocated_size << std::endl;
  }
}
