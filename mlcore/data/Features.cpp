#include "Features.h"

namespace mlcore
{
  /*
   * Feature
   */
  Feature::Feature(FeatureType t, const std::string& n)
  :
    type(t),
    name(n)
  {}
}
