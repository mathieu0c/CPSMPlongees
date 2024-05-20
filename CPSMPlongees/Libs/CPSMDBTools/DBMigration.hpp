#pragma once

#include <CPSMGlobals.hpp>
#include <QSqlDatabase>
#include <UpdtGlobal.hpp>
#include <cstdint>

/*
 * Define all the migration functions in the CPP FILE!
 * Use the macro CPSM_DECLARE_MIGRATION_STEP defined below
 * A migration function must return a boolean indicating if the migration was successful or not
 * A migration function provides the following parameters:
 *  (const updt::Version& from_version, const updt::Version& target_version, QSqlDatabase database)
 */

#define CPSM_DECLARE_MIGRATION_STEP(target_version_MAJOR, target_version_MINOR, target_version_PATCH)              \
  class MigrationStepTo##target_version_MAJOR##_##target_version_MINOR##_##target_version_PATCH {                  \
    static bool MigrationFunction(const updt::Version& from_version, const updt::Version& target_version,          \
                                  QSqlDatabase database);                                                          \
    static const MigrationStep& kMigrationStep;                                                                    \
  };                                                                                                               \
  const MigrationStep&                                                                                             \
      MigrationStepTo##target_version_MAJOR##_##target_version_MINOR##_##target_version_PATCH::kMigrationStep{     \
          RegisterMigrationStep(                                                                                   \
              {updt::Version{target_version_MAJOR, target_version_MINOR, target_version_PATCH},                    \
               MigrationStepTo##target_version_MAJOR##_##target_version_MINOR##_##target_version_PATCH::           \
                   MigrationFunction})};                                                                           \
  bool MigrationStepTo##target_version_MAJOR##_##target_version_MINOR##_##target_version_PATCH::MigrationFunction( \
      const updt::Version& from_version, const updt::Version& target_version, QSqlDatabase database)

namespace cpsm::db::migration {

enum class NeedsMigrationState : int32_t { kNoMigrationNeeded, kUpgradeNeeded, kDowngradeNeeded };

/*#########################*/
/*#                       #*/
/*#      Internal use     #*/
/*#                       #*/
/*#########################*/

struct MigrationStep {
  updt::Version target_version;
  std::function<bool(const updt::Version&, const updt::Version&, QSqlDatabase)> migration_function;
};

inline std::vector<MigrationStep>& GetMutableMigrationSteps() {
  static std::vector<MigrationStep> migration_steps{};
  return migration_steps;
}

inline const std::vector<MigrationStep>& GetMigrationSteps() {
  return GetMutableMigrationSteps();
}

inline const auto& RegisterMigrationStep(const MigrationStep& step) {
  SPDLOG_DEBUG("Registering migration step to <{}>", step.target_version);
  return GetMutableMigrationSteps().emplace_back(step);
}

/*#########################*/
/*#                       #*/
/*#     User functions    #*/
/*#                       #*/
/*#########################*/

std::vector<MigrationStep> GetOrderedMigrationSteps();

NeedsMigrationState VersionNeedsMigration(updt::Version current_version, updt::Version target_version);
bool MigrateDB(updt::Version from_version, updt::Version to_version, QSqlDatabase database);

}  // namespace cpsm::db::migration
