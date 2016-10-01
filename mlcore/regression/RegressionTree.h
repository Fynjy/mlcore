/*
 * RegressionTree.h
 *
 *  Created on: 2016-09-04
 *      Author: Anton Kochnev
 */

#ifndef REGRESSIONTREE_H_
#define REGRESSIONTREE_H_

namespace mlcore
{
  template<typename Examples>
  class RegressionTree
  {
  public:

    
    void learn(const Examples& examples);
  };
}

#endif /* REGRESSIONTREE_H_ */
