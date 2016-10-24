#ifndef MLCORE_TREE_CONTROL_H_
#define MLCORE_TREE_CONTROL_H_

#include <memory>
#include <vector>

#include "cppcore/thread/Executor.h"

#include "mlcore/data/VectorDataHistogram.h"
#include "mlcore/tree/Tree.h"

namespace mlcore
{
  double find_max(const std::vector<double>& vals, std::size_t& opt_inx);
  double find_max(const VectorDataPoint& vals, std::size_t points_count);

  class PointState
  {
  public:
    PointState(double max_orig, std::size_t point_count)
    :
      vals_(new double[point_count]),
      max_orig_(max_orig)
    {
      std::memset(vals_.get(), 0, point_count * sizeof(double));
    }

    PointState(PointState&& arg) = default;
    PointState(const PointState&) = delete;
    PointState& operator= (const PointState&) = delete;

    double operator[] (std::size_t inx) const
    {
      return vals_[inx];
    }

    double& operator[] (std::size_t inx)
    {
      return vals_[inx];
    }

    double max_orig() const
    {
      return max_orig_;
    }

  private:
    std::unique_ptr<double[]> vals_;
    double max_orig_;
  };

  typedef std::vector<PointState> PointStates;

  class VectorTreeLearner
  {
  public:
    VectorTreeLearner(VectorDataHistogram& examples);
    VectorForest learn();

  private:
    VectorDataHistogram& examples_;
    PointStates point_states_;
    cppcore::Executor executor_;

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

    double error(
      const std::deque<std::size_t>& region_indexes,
      const std::vector<double>& mean) const;

    static double error(
      const VectorDataPoint& points,
      const PointState& point_state,
      const std::vector<double>& mean,
      std::vector<double>& tmp);

    template<typename Comparator>
    void make_split(
      const std::deque<std::size_t>& region_indexes,
      Split& split,
      std::size_t split_point,
      Comparator cmp) const
    {
      std::vector<double> tmp(examples_.points_count());

      for (std::size_t inx : region_indexes)
      {
        if (cmp(examples_[inx].x(split.feature_num), split_point))
        {
          split.err1 += error(examples_[inx], point_states_[inx], split.m1, tmp);
        }
        else
        {
          split.err2 += error(examples_[inx], point_states_[inx], split.m2, tmp);
        }
      }

      split.err = split.err1 + split.err2;
    }
  };
}

#endif //MLCORE_TREE_CONTROL_H_
