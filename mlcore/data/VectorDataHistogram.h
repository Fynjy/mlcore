#ifndef MLCORE_GROUP_DATA_HISTORGAM_H_
#define MLCORE_GROUP_DATA_HISTORGAM_H_

#include <cstddef>
#include <deque>
#include <istream>
#include <ostream>

#include "Features.h"

namespace cppcore
{
  class MemoryPool;
}

namespace mlcore
{
  class VectorDataHistogram;

  class VectorDataPoint
  {
    friend VectorDataHistogram;

  public:
    VectorDataPoint(std::size_t* x, double* d);
    std::size_t x(std::size_t feature_num) const;
    std::size_t& x(std::size_t feature_num);
    double d(std::size_t point_num) const;
    double& d(std::size_t point_num);
    void d(std::vector<double>& vals) const;

  private:
    std::size_t* x_ = nullptr;
    double* d_;
  };

  class VectorDataHistogram
  {
  public:
    VectorDataHistogram();
    VectorDataHistogram(const Features& features, std::size_t points_count);
    VectorDataHistogram(VectorDataHistogram&& arg);
    void compile();
    void move(VectorDataHistogram& data_histogram, std::size_t b, std::size_t e);
    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save_as_text(std::ostream& os) const;
    void load_from_text(std::istream& is);

    std::size_t size() const;
    std::size_t features_count() const;
    std::size_t points_count() const;
    const DomainsPtr& domains() const;
    bool is_compiled() const;

    const VectorDataPoint& operator[] (std::size_t i) const;
    VectorDataPoint& operator[] (std::size_t i);
    void x(std::size_t inx, std::vector<double>& vals) const;
    double mark_to_val(std::size_t feature_num, std::size_t mark) const;
    std::size_t val_to_mark(std::size_t feature_num, double val);

    VectorDataPoint& add(const std::vector<double>& x);

    void trace_memory_stats(std::ostream& os);

  private:
    std::size_t points_count_ = 0;
    cppcore::MemoryPool memory_pool_;
    std::deque<VectorDataPoint> points_;
    DomainsPtr domains_;
    bool is_compiled_ = false;

  private:
    void push_back();
  };
}

#endif // MLCORE_GROUP_DATA_HISTORGAM_H_
