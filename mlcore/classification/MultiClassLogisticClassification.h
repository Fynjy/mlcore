#ifndef MLCORE_MULTI_CLASS_LOGISTIC_CLASSIFICATION_H_
#define MLCORE_MULTI_CLASS_LOGISTIC_CLASSIFICATION_H_

#include "cppcore/thread/Executor.h"

#include "mlcore/tree/Tree.h"

namespace mlcore
{
  class MultiClassLogisticClassification
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
    MultiClassLogisticClassification(const VectorDataHistogram& examples);
    VectorForest learn(LearnIterationCallback* callback = nullptr);
    const std::vector<double>& variable_importance() const;

  private:
    const VectorDataHistogram& examples_;
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
      MakeNodeTask(MultiClassLogisticClassification& l, VectorNodePtr& n)
        :
        learner(l),
        node(n)
      {}

      void execute()
      {
        node = learner.make_node(ctx);
      }

    public:
      MultiClassLogisticClassification& learner;
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
  };
}

#endif // MLCORE_MULTI_CLASS_LOGISTIC_CLASSIFICATION_H_
