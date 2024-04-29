#pragma once

#ifdef SPDLOG_ACTIVE_LEVEL
#undef SPDLOG_ACTIVE_LEVEL
#endif
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <spdlog/spdlog.h>

namespace logger {
constexpr auto kLoggerName{"Default"};
constexpr auto kLoggerPattern{R"(%H:%M:%S.%e [%^%l%$] %s:%# (%!): %v)"};

constexpr auto MaxLogLevel() {
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
  return spdlog::level::debug;
#else
  return spdlog::level::trace;
#endif
}

}  // namespace logger
