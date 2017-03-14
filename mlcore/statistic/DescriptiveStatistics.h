#ifndef MLCORE_STATISTIC_DESCRIPTIVE_STATISTICS_H_
#define MLCORE_STATISTIC_DESCRIPTIVE_STATISTICS_H_

#include <cstddef>

namespace mlcore
{
  class DescriptiveStatistics
  {
  public:
    double mean() const
    {
      if (n_ == 0)
      {
        return 0;
      }

      return (s_ / n_);
    }

    DescriptiveStatistics& operator+= (double arg)
    {
      s_ += arg;
      ++n_;
      s2_ += arg * arg;
      return *this;
    }

    double variance() const
    {
      if (n_ < 2)
      {
        return 0;
      }

      const double m = mean();
      const double v2 = (s2_ - m * m * n_) / (n_ - 1);
      return (v2 > 0 ? v2 : 0);
    }

    std::size_t count() const
    {
      return n_;
    }

    double sum() const
    {
      return s_;
    }

  private:
    double s_ = 0;
    std::size_t n_ = 0;
    double s2_ = 0;
  };
}

#endif // MLCORE_STATISTIC_DESCRIPTIVE_STATISTICS_H_
