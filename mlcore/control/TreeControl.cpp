#include <iostream>
#include <limits>
#include <sstream>
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

  namespace
  {
    std::string t_test_debug;
  }

  double t_test(const DescriptiveStatistics& val1, const DescriptiveStatistics& val2)
  {
    // p, %  99.99   99.90   99.00   97.72   97.50   95.00   90.00   84.13   50.00
    // q     3.715   3.090   2.326   2.000   1.960   1.645   1.282   1.000   0.000
    const double factor = val1.variance() / val1.count() + val2.variance() / val2.count();

    if (std::fabs(factor) < 1e-6)
    {
      return std::numeric_limits<double>::max();
    }

    std::stringstream oss;
    oss << "val1.mean() = " << val1.mean() << " val2.mean() = " << val2.mean()
      << " val1.variance() = " << val1.variance() << " val2.variance() = " << val2.variance()
      << " val1.count() = " << val1.count() << " val2.count() = " << val2.count()
      << " test = " << std::fabs(val1.mean() - val2.mean()) / std::sqrt(factor);

    t_test_debug = oss.str();
    return std::fabs(val1.mean() - val2.mean()) / std::sqrt(factor);
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

    void add(const PointState& point)
    {
      for (std::size_t i = 0; i < vals_.size(); ++i)
      {
        vals_[i] += point.r(i);
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
  VectorTreeLearner::VectorTreeLearner(const VectorDataHistogram& examples, LearnIterationCallback* callback)
  :
    examples_(examples),
    callback_(callback),
    //executor_(std::thread::hardware_concurrency() - 1)
    executor_(1)
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
    variable_importance_.resize(examples_.features_count());

    for (std::size_t i = 0; i < examples_.size(); ++i)
    {
      point_states_.emplace_back(examples_[i], points_count);
    }

    std::size_t iter_num = 0;

    while (flag)
    {
      VectorTree tree;

      if (flag = learn_iteration(tree))
      {
        trees.emplace_back(std::move(tree));

        if (callback_)
        {
          callback_->call(trees, iter_num++, variable_importance_);
        }
      }
    }

    return VectorForest(std::move(trees));
  }

  const std::vector<double>& VectorTreeLearner::variable_importance() const
  {
    return variable_importance_;
  }

  bool VectorTreeLearner::learn_iteration(VectorTree& tree)
  {
    Context ctx;
    ctx.m.resize(examples_.points_count(), 0);

    for (std::size_t i = 0; i < examples_.size(); ++i)
    {
      ctx.region_indexes.push_back(i);
    }

    ctx.err = error(ctx.region_indexes, ctx.m).sum();
    VectorNodePtr root = make_node(ctx);
    executor_.wait_for_empty();

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
      variable_importance_[split.feature_num] += (ctx.err - split.err);
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

      {
        std::cout << "BEST_SPLIT: " << examples_.domains()->at(split.feature_num).get_feature().name
          << " = " << examples_.mark_to_val(split.feature_num, split.split_point) << std::endl;

        if (left_task->ctx.region_indexes.size() < 100)
        {
          std::cout << "left_task->ctx.region_indexes = [";

          for (auto inx : left_task->ctx.region_indexes)
          {
            std::cout << inx << ',';
          }

          std::cout << "]\n";
        }

        if (right_task->ctx.region_indexes.size() < 100)
        {
          std::cout << "right_task->ctx.region_indexes = [";

          for (auto inx : right_task->ctx.region_indexes)
          {
            std::cout << inx << ',';
          }

          std::cout << "]\n";
        }
      }

      executor_.submit(std::bind(&MakeNodeTask::execute, left_task));
      executor_.submit(std::bind(&MakeNodeTask::execute, right_task));
      return VectorNodePtr(node.release());
    }

    if (ctx.region_indexes.size() == examples_.size())
    {
      return nullptr;
    }

    return make_terminal_node(ctx.m, ctx.region_indexes);
  }

  VectorNodePtr VectorTreeLearner::make_terminal_node(const std::vector<double>& d, const std::deque<std::size_t>& region_indexes)
  {
    for (std::size_t i : region_indexes)
    {
      const VectorDataPoint& point = examples_[i];

      for (std::size_t inx = 0; inx < d.size(); ++inx)
      {
        point_states_[i].d(inx) += d[inx];
        point_states_[i].r(inx) -= d[inx];
      }
    }

    std::ostringstream oss;
    oss << "terminal d = [";

    for (auto v : d)
    {
      oss << v << ';';
    }

    static std::size_t consumed_size = 0;
    consumed_size += region_indexes.size();
    oss << "] " << region_indexes.size() << " (" << consumed_size << " from " << examples_.size() << ")" << std::endl;
    std::cout << oss.str();

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
      vals[e.x(feature_num)].add(point_states_[inx]);
    }

    VectorMean val_all(examples_.points_count());

    for (std::size_t i = 0; i < vals.size(); ++i)
    {
      val_all.add(vals[i]);
    }

    VectorMean val1(examples_.points_count());
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

      if (val1.n() >= 2 && val2.n() >= 2)
      {
        Split s;
        s.m1 = val1.get();
        s.m2 = val2.get();
        s.feature_num = feature_num;
        s.split_point = split_point;

        if (domain.get_feature().type == mlcore::FeatureType::NotOrdered)
        {
          make_split(region_indexes, s, compare_unordered);
        }
        else
        {
          make_split(region_indexes, s, compare_ordered);
        }

        if (s.has_split && s.err < split.err)
        {
          std::cout << domain.get_feature().name << " = " << examples_.mark_to_val(feature_num, split_point)
            << " s.err = " << std::fixed << s.err << " split.err = " << std::fixed << split.err << std::endl;
          std::cout << t_test_debug << std::endl;

          split = s;
        }
      }
    }
  }

  DescriptiveStatistics VectorTreeLearner::error(
    const std::deque<std::size_t>& region_indexes,
    const std::vector<double>& mean) const
  {
    DescriptiveStatistics err;

    for (std::size_t inx : region_indexes)
    {
      err += error(examples_[inx], point_states_[inx], mean);
    }

    return err;
  }

  double VectorTreeLearner::error(
    const VectorDataPoint& point,
    const PointState& point_state,
    const std::vector<double>& mean)
  {
    double max_val = std::numeric_limits<double>::lowest();
    std::size_t opt_inx = 0;

    for (std::size_t i = 0; i < mean.size(); ++i)
    {
      const double tmp = point_state.d(i) + mean[i];

      if (max_val < tmp)
      {
        max_val = tmp;
        opt_inx = i;
      }
    }

    return (point_state.max_orig() - point.d(opt_inx));
  }
}
