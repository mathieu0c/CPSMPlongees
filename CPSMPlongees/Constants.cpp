#include "Constants.hpp"

namespace consts::colors {

QColor GetColorForDivingType(int diving_type_id, int default_diving_type_id) {
  switch (diving_type_id) {
    case 1:
      return kBackgroundCyan;
    case 2:
      return kBackgroundPurple;
    default:
      break;
  }
  if (diving_type_id == default_diving_type_id) return kCriticalRed;
  ;
  return {};
}

}  // namespace consts::colors
