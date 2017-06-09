#include "MultiClassLogisticClassification.h"

namespace mlcore
{
  MultiClassLogisticClassification::MultiClassLogisticClassification(const VectorDataHistogram& examples)
  :
    examples_(examples),
    executor_(std::thread::hardware_concurrency() - 1)
  {
    if (!examples.is_compiled())
    {
      throw std::runtime_error("VectorDataHistogram don't compiled");
    }
  }

  VectorForest MultiClassLogisticClassification::learn(LearnIterationCallback* callback)
  {
    std::vector<VectorTree> trees;
    return VectorForest(std::move(trees));
  }

  const std::vector<double>& MultiClassLogisticClassification::variable_importance() const
  {
    return variable_importance_;
  }

  bool MultiClassLogisticClassification::learn_iteration(VectorTree& tree)
  {
    return false;
  }

  VectorNodePtr MultiClassLogisticClassification::make_node(const Context& ctx)
  {
    return VectorNodePtr();
  }

  VectorNodePtr MultiClassLogisticClassification::make_terminal_node(const std::vector<double>& d, const std::deque<std::size_t>& region_indexes)
  {
    return VectorNodePtr();
  }

  void MultiClassLogisticClassification::best_split(
    std::size_t feature_num,
    Split& split,
    const std::deque<std::size_t>& region_indexes) const
  {

  }
}
