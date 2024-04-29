#pragma once

#include <cstdint>

namespace btype {

enum class BuildType_t : int32_t { kDebug = 0, kRelWithDebInfo = 1, kRelease = 2, kMinSizeRel = 3 };

constexpr auto* to_string(BuildType_t btype) {
  switch (btype) {
    case BuildType_t::kDebug:
      return "Debug";
      break;
    case BuildType_t::kRelWithDebInfo:
      return "RelWithDebInfo";
      break;
    case BuildType_t::kRelease:
      return "Release";
      break;
    case BuildType_t::kMinSizeRel:
      return "MinSizeRel";
      break;
    default:
      break;
  }
  return "Unknown";
}

constexpr auto BuildType() {
  return static_cast<BuildType_t>(CMAKE_COMPIL_MODE);
}
constexpr auto BuildTypeStr() {
  return to_string(BuildType());
}

constexpr bool operator==(BuildType_t lh, BuildType_t rh) {
  return static_cast<int32_t>(lh) == static_cast<int32_t>(rh);
}

constexpr bool HasDebInfo() {
  return BuildType() == BuildType_t::kDebug || BuildType() == BuildType_t::kRelWithDebInfo;
}

}  // namespace btype
