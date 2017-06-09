#pragma once

#include "mlcore/data/DataHistogram.h"
#include "mlcore/tree/Tree.h"

namespace mlcore
{
  class RootMeanSquaredLogarithmicError
  {
  public:
    RootMeanSquaredLogarithmicError(const DataHistogram& examples);
    ScalarForest learn();
  };
}
