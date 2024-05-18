#pragma once

#include <QStandardPaths>

#include "CpsmGlobalConfiguration.hpp"

namespace cpsm::consts {

#ifdef CPSM_BUILD_AS_MOCKUP
constexpr bool kIsBuiltAsMockup{true};
#else
constexpr bool kIsBuiltAsMockup{false};
#endif

// -- Paths

const auto kAppDataPath{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
const auto kCPSMDbFile{kIsBuiltAsMockup ? QString{"CPSMData_MOCKUP.db"} : QString{"CPSMData.db"}};
const auto kCPSMDbPath{QString{"%0/%1"}.arg(kAppDataPath, kCPSMDbFile)};

//-- format

constexpr auto kDateFormat{"yyyy-MM-dd"};
constexpr auto kTimeFormat{"HH:mm:ss"};

}  // namespace cpsm::consts
