#include <sstream>
#include <stdexcept>

#include <cppcore/memory/MemoryPool.h>
#include "cppcore/memory/MemoryPoolAllocator.h"
#include "cppcore/io/Serialization.h"

#include "DataHistogram.h"

namespace mlcore
{
  /*
   * DataPoint
   */
  DataPoint::DataPoint(std::size_t* x, double d)
  :
    x_(x),
    d_(d)
  {}

  std::size_t DataPoint::x(std::size_t feature_num) const
  {
    return x_[feature_num];
  }

  std::size_t& DataPoint::x(std::size_t feature_num)
  {
    return x_[feature_num];
  }

  double DataPoint::d() const
  {
    return d_;
  }

  double& DataPoint::d()
  {
    return d_;
  }

  /*
   * DataHistogram
   */
  DataHistogram::DataHistogram()
  :
    memory_pool_(1024 * 64)
  {}

  DataHistogram::DataHistogram(const Features& features, std::size_t points_count)
  :
    memory_pool_(1024 * 64),
    domains_(std::make_shared<Domains>(cppcore::MemoryPoolAllocatorType(memory_pool_), features))
  {}

  DataHistogram::DataHistogram(DataHistogram&& arg)
  :
    memory_pool_(std::move(arg.memory_pool_)),
    points_(std::move(arg.points_)),
    domains_(std::make_shared<Domains>(cppcore::MemoryPoolAllocatorType(memory_pool_), arg.domains_->features()))
  {}

  void DataHistogram::compile()
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
      auto& point = points_[j];

      for (std::size_t i = 0; i < features_count(); ++i)
      {
        point.x(i) = renum_maps[i][point.x(i)];
      }
    }

    is_compiled_ = true;
  }

  void DataHistogram::move(DataHistogram& data_histogram, std::size_t b, std::size_t e)
  {
    data_histogram.points_.insert(
      data_histogram.points_.end(),
      points_.begin() + b,
      points_.begin() + e);
    points_.erase(points_.begin() + b, points_.begin() + e);
    data_histogram.domains_ = domains_;
    data_histogram.is_compiled_ = is_compiled_;
  }

  void DataHistogram::save(std::ostream& os) const
  {
    cppcore::write(os, features_count());

    for (const auto& domain : *domains_)
    {
      domain.save(os);
    }

    cppcore::write(os, points_.size());

    for (std::size_t i = 0; i < points_.size(); ++i)
    {
      os.write(reinterpret_cast<const char*>(points_[i].x_), features_count() * sizeof(std::size_t));
      os.write(reinterpret_cast<const char*>(&points_[i].d_), sizeof(double));
    }
  }

  void DataHistogram::push_back()
  {
    cppcore::MemoryPoolAllocator<char> alloc(memory_pool_);
    char* x = alloc.allocate(features_count() * sizeof(std::size_t));
    points_.emplace_back(reinterpret_cast<std::size_t*>(x), 0);
  }

  void DataHistogram::load(std::istream& is)
  {
    std::size_t sz;
    cppcore::read(is, sz);
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
      is.read(reinterpret_cast<char*>(&points_.back().d_), sizeof(double));
    }

    is_compiled_ = true;
  }

  void DataHistogram::save_as_text(std::ostream& os) const
  {
    os << features_count() << " # features_count\n";

    for (const auto& domain : *domains_)
    {
      os << domain.get_feature().name << ' ' << static_cast<int>(domain.get_feature().type) << '\n';
    }

    os << size() << " # size\n";

    for (std::size_t i = 0; i < points_.size(); ++i)
    {
      const auto& point = points_[i];

      for (std::size_t inx = 0; inx < features_count(); ++inx)
      {
        os << mark_to_val(inx, point.x(inx)) << ' ';
      }

      os << "| " << point.d() << ' ' << '\n';
    }
  }

  void DataHistogram::load_from_text(std::istream& is)
  {
    std::size_t sz = 0;
    std::string tmp;
    is >> sz;
    std::getline(is, tmp);
    Features features;

    for (std::size_t i = 0; i < sz; ++i)
    {
      std::string name;
      int type = 0;
      is >> name >> type;
      features.emplace_back(FeatureType(type), name);
    }

    domains_ = std::make_shared<Domains>(cppcore::MemoryPoolAllocatorType(memory_pool_), features);
    std::getline(is, tmp);
    is >> sz;
    std::getline(is, tmp);
    std::vector<double> x(features_count(), 0);

    for (std::size_t i = 0; i < sz; ++i)
    {
      for (std::size_t inx = 0; inx < features_count(); ++inx)
      {
        is >> x[inx];
      }

      char ch;
      is >> ch;
      auto& point = add(x);
      is >> point.d();
    }

    compile();
  }

  std::size_t DataHistogram::size() const
  {
    return points_.size();
  }

  std::size_t DataHistogram::features_count() const
  {
    return domains_->size();
  }

  const DomainsPtr& DataHistogram::domains() const
  {
    return domains_;
  }

  bool DataHistogram::is_compiled() const
  {
    return is_compiled_;
  }

  const DataPoint& DataHistogram::operator[] (std::size_t i) const
  {
    return points_[i];
  }

  DataPoint& DataHistogram::operator[] (std::size_t i)
  {
    return points_[i];
  }

  void DataHistogram::x(std::size_t inx, std::vector<double>& vals) const
  {
    const auto& point = points_[inx];

    for (std::size_t i = 0; i < vals.size(); ++i)
    {
      vals[i] = mark_to_val(i, point.x(i));
    }
  }

  double DataHistogram::mark_to_val(std::size_t feature_num, std::size_t mark) const
  {
    return (*domains_)[feature_num].to_val(mark);
  }

  std::size_t DataHistogram::val_to_mark(std::size_t feature_num, double val)
  {
    return (*domains_)[feature_num].to_mark(val);
  }

  DataPoint& DataHistogram::add(const std::vector<double>& x)
  {
    push_back();

    for (std::size_t i = 0; i < features_count(); ++i)
    {
      points_.back().x(i) = val_to_mark(i, x[i]);
    }

    return points_.back();
  }

  DataPoint& DataHistogram::addFrom(const DataHistogram& examples, std::size_t i)
  {
    const auto src = examples[i];
    push_back();
    auto& trg = points_.back();

    for (std::size_t i = 0; i < features_count(); ++i)
    {
      trg.x(i) = val_to_mark(i, examples.mark_to_val(i, src.x(i)));
    }

    trg.d() = src.d();
    return points_.back();
  }

  void DataHistogram::trace_memory_stats(std::ostream& os)
  {
    const auto debug_info = memory_pool_.get_debug_info();
    os << "Memory pool stats:\n"
      << "allocation_count = " << debug_info.allocation_count << "\n"
      << "allocated_size = " << debug_info.allocated_size << std::endl;
  }
}
