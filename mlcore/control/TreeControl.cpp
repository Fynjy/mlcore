#include <limits>
#include <stdexcept>

#include "TreeControl.h"

namespace mlcore
{
  double find_max(const std::vector<double>& vals, std::size_t& opt_inx)
  {
    double max_val = vals[0];
    opt_inx = 0;

    for (std::size_t i = 1; i < vals.size(); ++i)
    {
      if (max_val < vals[i])
      {
        max_val = vals[i];
        opt_inx = i;
      }
    }

    return max_val;
  }

  double find_max(const VectorDataPoint& vals, std::size_t points_count)
  {
    double max_val = vals.d(0);

    for (std::size_t i = 1; i < points_count; ++i)
    {
      if (max_val < vals.d(i))
      {
        max_val = vals.d(i);
      }
    }

    return max_val;
  }

  /*
   * VectorMean
   */
  class VectorMean
  {
  public:
    VectorMean(std::size_t points_count)
    :
      vals_(points_count, 0)
    {}

    void add(const VectorDataPoint& point)
    {
      for (std::size_t i = 0; i < vals_.size(); ++i)
      {
        vals_[i] += point.d(i);
      }

      ++n_;
    }

    void add(const VectorMean& arg)
    {
      for (std::size_t i = 0; i < vals_.size(); ++i)
      {
        vals_[i] += arg.vals_[i];
      }

      n_ += arg.n_;
    }

    void minus(const VectorMean& arg)
    {
      for (std::size_t i = 0; i < vals_.size(); ++i)
      {
        vals_[i] -= arg.vals_[i];
      }

      n_ -= arg.n_;
    }

    void get(std::vector<double>& m) const
    {
      m.resize(vals_.size());

      for (std::size_t i = 0; i < vals_.size(); ++i)
      {
        m[i] = vals_[i] / n_;
      }
    }

    std::vector<double> get() const
    {
      std::vector<double> m;
      get(m);
      return m;
    }

    std::size_t n() const
    {
      return n_;
    }

  private:
    std::vector<double> vals_;
    std::size_t n_ = 0;
  };

  /*
   * VectorTreeLearner
   */
  VectorTreeLearner::VectorTreeLearner(VectorDataHistogram& examples)
  :
    examples_(examples),
    executor_(std::thread::hardware_concurrency() - 1)
  {
    if (!examples.is_compiled())
    {
      throw std::runtime_error("VectorDataHistogram don't compiled");
    }
  }

  VectorForest VectorTreeLearner::learn()
  {
    std::vector<VectorTree> trees;
    bool flag = true;
    point_states_.clear();
    point_states_.reserve(examples_.size());
    const std::size_t points_count = examples_.points_count();

    for (std::size_t i = 0; i < examples_.size(); ++i)
    {
      point_states_.emplace_back(find_max(examples_[i], points_count), points_count);
    }

    while (flag)
    {
      VectorTree tree;

      if (flag = learn_iteration(tree))
      {
        trees.emplace_back(std::move(tree));
      }
    }

    return VectorForest(std::move(trees));
  }

  bool VectorTreeLearner::learn_iteration(VectorTree& tree)
  {
    Context ctx;
    VectorMean m(examples_.points_count());

    for (std::size_t i = 0; i < examples_.size(); ++i)
    {
      ctx.region_indexes.push_back(i);
      m.add(examples_[i]);
    }

    m.get(ctx.m);
    ctx.err = error(ctx.region_indexes, ctx.m);
    cppcore::Executor executor(std::thread::hardware_concurrency() - 1);
    VectorNodePtr root = make_node(ctx);
    executor.wait_for_empty();

    if (root)
    {
      tree = VectorTree(root.release());
      return true;
    }

    return false;
  }

  namespace
  {
    bool compare_ordered(std::size_t val1, std::size_t val2)
    {
      return (val1 <= val2);
    }

    bool compare_unordered(std::size_t val1, std::size_t val2)
    {
      return (val1 == val2);
    }
  }

  VectorNodePtr VectorTreeLearner::make_node(const Context& ctx)
  {
    const std::size_t feature_count = examples_.features_count();
    Split split;
    split.err = ctx.err;
    split.criteria_value = 0;

    for (std::size_t n = 0; n < feature_count; ++n)
    {
      best_split(n, split, ctx.region_indexes);
    }

    if (split.has_split)
    {
      auto node = std::make_unique<NonTerminalNode<std::vector<double>>>();
      node->filter.feature_num = split.feature_num;
      node->filter.split_point = examples_.mark_to_val(split.feature_num, split.split_point);
      const FeatureType feature_type = examples_.domains()->at(split.feature_num).get_feature().type;
      node->filter.feature_type = feature_type;
      auto left_task = std::make_shared<MakeNodeTask>(*this, node->left);
      auto right_task = std::make_shared<MakeNodeTask>(*this, node->right);

      left_task->ctx.m = split.m1;
      left_task->ctx.err = split.err1;
      right_task->ctx.m = split.m2;
      right_task->ctx.err = split.err2;

      const auto cmp = (feature_type == mlcore::FeatureType::NotOrdered ? compare_unordered : compare_ordered);

      for (std::size_t i = 0; i < ctx.region_indexes.size(); ++i)
      {
        const std::size_t inx = ctx.region_indexes[i];

        if (cmp(examples_[inx].x(split.feature_num), split.split_point))
        {
          left_task->ctx.region_indexes.push_back(inx);
        }
        else
        {
          right_task->ctx.region_indexes.push_back(inx);
        }
      }

      executor_.submit(std::bind(&MakeNodeTask::execute, left_task));
      executor_.submit(std::bind(&MakeNodeTask::execute, right_task));
      return VectorNodePtr(node.release());
    }

    return make_terminal_node(ctx.m, ctx.region_indexes);
  }

  VectorNodePtr VectorTreeLearner::make_terminal_node(const std::vector<double>& d, const std::deque<std::size_t>& region_indexes)
  {
    for (std::size_t i : region_indexes)
    {
      VectorDataPoint& point = examples_[i];

      for (std::size_t inx = 0; inx < d.size(); ++inx)
      {
        point_states_[i][inx] += d[inx];
        point.d(inx) -= d[inx];
      }
    }

    return std::make_unique<TerminalNode<std::vector<double>>>(d);
  }

  void VectorTreeLearner::best_split(
    std::size_t feature_num,
    Split& split,
    const std::deque<std::size_t>& region_indexes) const
  {
    const auto& domain = examples_.domains()->at(feature_num);

    if (domain.size() <= 1)
    {
      return;
    }

    std::vector<VectorMean> vals(domain.size(), VectorMean(examples_.points_count()));

    for (auto inx : region_indexes)
    {
      const auto& e = examples_[inx];
      vals[e.x(feature_num)].add(e);
    }

    VectorMean val_all(domain.size());

    for (std::size_t i = 0; i < vals.size(); ++i)
    {
      val_all.add(vals[i]);
    }

    VectorMean val1(domain.size());
    VectorMean val2 = val_all;

    for (std::size_t split_point = 0; split_point < vals.size(); ++split_point)
    {
      if (!vals[split_point].n())
      {
        continue;
      }

      if (domain.get_feature().type == mlcore::FeatureType::NotOrdered)
      {
        val1 = vals[split_point];
        val2 = val_all;
        val2.minus(vals[split_point]);
      }
      else
      {
        val1.add(vals[split_point]);
        val2.minus(vals[split_point]);
      }

      Split s;
      s.m1 = val1.get();
      s.m2 = val2.get();
      s.feature_num = feature_num;

      if (domain.get_feature().type == mlcore::FeatureType::NotOrdered)
      {
        make_split(region_indexes, s, split_point, compare_unordered);
      }
      else
      {
        make_split(region_indexes, s, split_point, compare_ordered);
      }

      //double criteria_value = 0;
      //const bool pass_test = t_test(val1, val2, criteria_value);

      //if (criteria_value > split.criteria_value &&
      //    pass_test)
      {
        //split.criteria_value = criteria_value;

        if (s.err < split.err)
        {
          split.err = s.err;
          split.feature_num = feature_num;
          split.split_point = split_point;
          split.has_split = true;
          split.m1 = s.m1;
          split.m2 = s.m2;
          split.err1 = s.err1;
          split.err2 = s.err2;
        }
      }
    }
  }

  double VectorTreeLearner::error(
    const std::deque<std::size_t>& region_indexes,
    const std::vector<double>& mean) const
  {
    std::vector<double> tmp(examples_.points_count());
    double err = 0;

    for (std::size_t inx : region_indexes)
    {
      err += error(examples_[inx], point_states_[inx], mean, tmp);
    }

    return err;
  }

  double VectorTreeLearner::error(
    const VectorDataPoint& points,
    const PointState& point_state,
    const std::vector<double>& mean,
    std::vector<double>& tmp)
  {
    for (std::size_t j = 0; j < tmp.size(); ++j)
    {
      tmp[j] = point_state[j] + mean[j];
    }

    std::size_t opt_inx = 0;
    const double r = find_max(tmp, opt_inx);
    return (point_state.max_orig() - r);
  }
}
