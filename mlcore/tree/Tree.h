#ifndef MLCORE_TREE_TREE_H_
#define MLCORE_TREE_TREE_H_

#include <vector>

#include "Node.h"

namespace mlcore
{
  template<typename T>
  class Tree
  {
  public:
    Tree()
    {}

    Tree(Node<T>* root)
    :
      root_(root)
    {}

    const T& value(const std::vector<double>& x) const
    {
      return root_->value(x);
    }

    void save(std::ostream& os) const
    {
      root_->save(os);
    }

    void load(std::istream& is)
    {
      root_ = Node<T>::load_node(is);
    }

  private:
    std::unique_ptr<Node<T>> root_;
  };

  typedef Tree<double> ScalarTree;
  typedef Tree<std::vector<double>> VectorTree;

  template<typename T>
  class Forest
  {
  public:
    typedef Tree<T> TreeType;

  public:
    Forest()
    {}

    Forest(std::vector<TreeType>&& trees)
    :
      trees_(std::move(trees))
    {}

    const T& value(const std::vector<double>& x) const
    {
      auto it = trees_.begin();
      T res = it->value(x);
      ++it;

      while (it != trees_.end())
      {
        ValueHelper::add(res, it->value(x));
        ++it;
      }

      return res;
    }

    void save(std::ostream& os) const
    {
      os << tree_.size() << ' ';

      for (const auto& tree : trees_)
      {
        tree.save(os);
      }
    }

    void load(std::istream& is)
    {
      std::size_t sz = 0;
      is >> sz;
      trees_.resize(sz);

      for (std::size_t i = 0; i < sz; ++i)
      {
        trees_[i].load(is);
      }
    }

  private:
    std::vector<TreeType> trees_;
  };

  typedef Forest<double> ScalarForest;
  typedef Forest<std::vector<double>> VectorForest;
}

#endif // MLCORE_TREE_TREE_H_
