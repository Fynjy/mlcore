#ifndef MLCORE_TREE_CONTROL_H_
#define MLCORE_TREE_CONTROL_H_

#include <memory>
#include <vector>

#include "cppcore/thread/Executor.h"

#include "mlcore/data/VectorDataHistogram.h"
#include "mlcore/statistic/DescriptiveStatistics.h"
#include "mlcore/tree/Tree.h"

namespace mlcore
{
  double find_max(const std::vector<double>& vals, std::size_t& opt_inx);
  double find_max(const VectorDataPoint& vals, std::size_t points_count);
  double t_test(const DescriptiveStatistics& val1, const DescriptiveStatistics& val2);

  class PointState
  {
  public:
    PointState(const VectorDataPoint& point, std::size_t points_count)
    :
      r_(new double[points_count]),
      d_(new double[points_count]),
      max_orig_(find_max(point, points_count))
    {
      for (std::size_t i = 0; i < points_count; ++i)
      {
        r_[i] = point.d(i);
      }

      std::memset(d_.get(), 0, points_count * sizeof(double));
    }

    PointState(PointState&& arg) = default;
    PointState(const PointState&) = delete;
    PointState& operator= (const PointState&) = delete;

    double d(std::size_t inx) const
    {
      return d_[inx];
    }

    double& d(std::size_t inx)
    {
      return d_[inx];
    }

    double r(std::size_t inx) const
    {
      return r_[inx];
    }

    double& r(std::size_t inx)
    {
      return r_[inx];
    }

    double max_orig() const
    {
      return max_orig_;
    }

  private:
    std::unique_ptr<double[]> d_;
    std::unique_ptr<double[]> r_;
    double max_orig_;
  };

  typedef std::vector<PointState> PointStates;

  class VectorTreeLearner
  {
  public:
    class LearnIterationCallback
    {
    public:
      virtual ~LearnIterationCallback()
      {}

      virtual void call(
        const std::vector<VectorTree>& trees,
        std::size_t iter_num,
        const std::vector<double>& variable_importance) = 0;
    };

  public:
    VectorTreeLearner(const VectorDataHistogram& examples, LearnIterationCallback* callback = nullptr);
    VectorForest learn();
    const std::vector<double>& variable_importance() const;

  private:
    const VectorDataHistogram& examples_;
    LearnIterationCallback* callback_;
    PointStates point_states_;
    cppcore::Executor executor_;
    std::vector<double> variable_importance_;

  private:
    struct Context
    {
      std::deque<std::size_t> region_indexes;
      double err = 0;
      std::vector<double> m;
    };

    struct Split
    {
      bool has_split = false;
      double err = 0;
      std::vector<double> m1;
      std::vector<double> m2;
      double err1 = 0;
      double err2 = 0;
      double criteria_value = 0;
      std::size_t feature_num = 0;
      std::size_t split_point = 0;
    };

    class MakeNodeTask
    {
    public:
      MakeNodeTask(VectorTreeLearner& l, VectorNodePtr& n)
      :
        learner(l),
        node(n)
      {}

      void execute()
      {
        node = learner.make_node(ctx);
      }

    public:
      VectorTreeLearner& learner;
      Context ctx;
      VectorNodePtr& node;
    };

  private:
    bool learn_iteration(VectorTree& tree);
    VectorNodePtr make_node(const Context& ctx);
    VectorNodePtr make_terminal_node(const std::vector<double>& d, const std::deque<std::size_t>& region_indexes);

    void best_split(
      std::size_t feature_num,
      Split& split,
      const std::deque<std::size_t>& region_indexes) const;

    DescriptiveStatistics error(
      const std::deque<std::size_t>& region_indexes,
      const std::vector<double>& mean) const;

    static double error(
      const VectorDataPoint& points,
      const PointState& point_state,
      const std::vector<double>& mean);

    template<typename Comparator>
    void make_split(
      const std::deque<std::size_t>& region_indexes,
      Split& split,
      Comparator cmp) const
    {
      DescriptiveStatistics err1;
      DescriptiveStatistics err2;

      for (std::size_t inx : region_indexes)
      {
        if (cmp(examples_[inx].x(split.feature_num), split.split_point))
        {
          err1 += error(examples_[inx], point_states_[inx], split.m1);
        }
        else
        {
          err2 += error(examples_[inx], point_states_[inx], split.m2);
        }
      }

      split.has_split = (t_test(err1, err2) > 2);
      split.err1 = err1.sum();
      split.err2 = err2.sum();
      split.err = split.err1 + split.err2;
    }
  };
}

#endif //MLCORE_TREE_CONTROL_H_
