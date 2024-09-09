#pragma once

#include <QDate>
#include <QStandardPaths>
#include <UpdtGlobal.hpp>

#include "CpsmGlobalConfiguration.hpp"

namespace cpsm::consts {

/*#########################*/
/*#                       #*/
/*#   Project constants   #*/
/*#                       #*/
/*#########################*/

constexpr auto kProjectGithubRelease{PROJECT_GITHUB_RELEASE_DEFINE};
constexpr auto kAppName{"CPSMPlongees"};

constexpr updt::Version kCurrentVersion{
    MAIN_PROJECT_VERSIONING_MAJOR, MAIN_PROJECT_VERSIONING_MINOR, MAIN_PROJECT_VERSIONING_PATCH};

/*#########################*/
/*#                       #*/
/*#         Build         #*/
/*#                       #*/
/*#########################*/

#ifdef CPSM_BUILD_AS_MOCKUP
constexpr bool kIsBuiltAsMockup{true};
#else
constexpr bool kIsBuiltAsMockup{false};
#endif

/*#########################*/
/*#                       #*/
/*#         Paths         #*/
/*#                       #*/
/*#########################*/

const auto kAppDataPath{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + kAppName};
const auto kCPSMDbFile{kIsBuiltAsMockup ? QString{"CPSMData_MOCKUP.db"} : QString{"CPSMData.db"}};
const auto kCPSMDbPath{QString{"%0/%1"}.arg(kAppDataPath, kCPSMDbFile)};
const auto kDBBackupPath{QString{"%0/DBBackup"}.arg(kAppDataPath)};

/*#########################*/
/*#                       #*/
/*#        Update         #*/
/*#                       #*/
/*#########################*/

constexpr auto kPublicVerifierKeyFile{"Assets/CPSMPlongees.public"};
constexpr auto kPostUpdateCommand{"CPSMPlongees.exe"};

/*#########################*/
/*#                       #*/
/*#        Format         #*/
/*#                       #*/
/*#########################*/

constexpr auto kDateUserFormat{"dd/MM/yyyy"};
constexpr auto kTimeFormat{"HH:mm"};

/*#########################*/
/*#                       #*/
/*#     Default values    #*/
/*#                       #*/
/*#########################*/

const QDate kEpochDate{1970, 1, 1};

}  // namespace cpsm::consts
