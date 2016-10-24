#ifndef MLCORE_TREE_NODE_H_
#define MLCORE_TREE_NODE_H_

#include <memory>

#include "mlcore/data/VectorDataHistogram.h"

#include "Filter.h"

namespace mlcore
{
  template<typename T>
  class Node
  {
  public:
    virtual ~Node() {}
    virtual const T& value(const std::vector<double>& x) const = 0;
    virtual void save(std::ostream& os) const = 0;
    virtual void load(std::istream& is) = 0;

    std::unique_ptr<Node> load_node(std::istream& is);
  };

  template<typename T>
  class ValueHelper
  {
  public:
    static void save(std::ostream& os, const T& val)
    {
      static_assert(false);
    }

    static void load(std::istream& is, T& val)
    {
      static_assert(false);
    }

    static T& add(T& arg1, const T& arg2)
    {
      static_assert(false);
    }
  };

  template<typename T>
  class TerminalNode
  :
    public Node<T>
  {
  public:
    static const char TYPE = 'T';

  public:
    TerminalNode()
    {}

    TerminalNode(const T& d_val)
    :
      d(d_val)
    {}

    virtual ~TerminalNode()
    {}

    virtual const T& value(const std::vector<double>&) const
    {
      return d;
    }

    virtual void save(std::ostream& os) const
    {
      os << TYPE << ' ';
      ValueHelper<T>::save(os, d);
      os << std::endl;
    }

    virtual void load(std::istream& is)
    {
      ValueHelper<T>::load(is, d);
    }

  public:
    std::vector<double> d;
  };

  template<typename T>
  class NonTerminalNode
  :
    public Node<T>
  {
  public:
    static const char TYPE = 'N';

  public:
    virtual ~NonTerminalNode()
    {}

    virtual const T& value(const std::vector<double>& x) const
    {
      if (filter.match(x))
      {
        return left->value(x);
      }

      return right->value(x);
    }

    virtual void save(std::ostream& os) const
    {
      os << TYPE << ' ';
      filter.save(os);
      left->save(os);
      right->save(os);
    }

    virtual void load(std::istream& is)
    {
      filter.load(is);
      left = load_node(is);
      right = load_node(is);
    }

  public:
    Filter filter;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
  };

  template<>
  class ValueHelper<double>
  {
  public:
    static void save(std::ostream& os, const double& val)
    {
      os << val;
    }

    static void load(std::istream& is, double& val)
    {
      is >> val;
    }
  };

  typedef Node<double> ScalarNode;
  typedef std::unique_ptr<ScalarNode> ScalarNodePtr;

  template<>
  class ValueHelper<std::vector<double>>
  {
  public:
    static void save(std::ostream& os, const std::vector<double>& val)
    {
      os << val.size() << ' ';

      for (double v : val)
      {
        os << v << ' ';
      }
    }

    static void load(std::istream& is, std::vector<double>& val)
    {
      std::size_t sz = 0;
      is >> sz;
      val.resize(sz);

      for (std::size_t i = 0; i < sz; ++i)
      {
        is >> val[i];
      }
    }
  };

  typedef Node<std::vector<double>> VectorNode;
  typedef std::unique_ptr<VectorNode> VectorNodePtr;
}

#endif // MLCORE_TREE_NODE_H_
