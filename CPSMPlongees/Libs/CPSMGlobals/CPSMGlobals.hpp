#pragma once

#include <QStandardPaths>

namespace cpsm::consts {

// -- Paths

const auto kAppDataPath{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
const auto kCPSMDbFile{QString{"CPSMData.db"}};
const auto kCPSMDbPath{QString{"%0/%1"}.arg(kAppDataPath, kCPSMDbFile)};

//-- format

constexpr auto kDateFormat{"yyyy-MM-dd"};
constexpr auto kTimeFormat{"HH:mm:ss"};

}  // namespace cpsm::consts
