#pragma once

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
  class DataHistogram;

  class DataPoint
  {
    friend DataHistogram;

  public:
    DataPoint(std::size_t* x, double d);
    std::size_t x(std::size_t feature_num) const;
    std::size_t& x(std::size_t feature_num);
    double d() const;
    double& d();

  private:
    std::size_t* x_ = nullptr;
    double d_;
  };

  class DataHistogram
  {
  public:
    DataHistogram();
    DataHistogram(const Features& features, std::size_t points_count);
    DataHistogram(DataHistogram&& arg);
    void compile();
    void move(DataHistogram& data_histogram, std::size_t b, std::size_t e);
    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save_as_text(std::ostream& os) const;
    void load_from_text(std::istream& is);

    std::size_t size() const;
    std::size_t features_count() const;
    const DomainsPtr& domains() const;
    bool is_compiled() const;

    const DataPoint& operator[] (std::size_t i) const;
    DataPoint& operator[] (std::size_t i);
    void x(std::size_t inx, std::vector<double>& vals) const;
    double mark_to_val(std::size_t feature_num, std::size_t mark) const;
    std::size_t val_to_mark(std::size_t feature_num, double val);

    DataPoint& add(const std::vector<double>& x);
    DataPoint& addFrom(const DataHistogram& examples, std::size_t i);

    void trace_memory_stats(std::ostream& os);

  private:
    cppcore::MemoryPool memory_pool_;
    std::deque<DataPoint> points_;
    DomainsPtr domains_;
    bool is_compiled_ = false;

  private:
    void push_back();
  };
}
