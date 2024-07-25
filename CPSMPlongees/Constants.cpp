#include "Constants.hpp"

namespace consts::colors {

QColor GetColorForDivingType(int diving_type_id) {
  switch (diving_type_id) {
    case 1:
      return kBackgroundCyan;
    case 2:
      return kBackgroundPurple;
    default:
      break;
  }
  return {};
}

}  // namespace consts::colors
