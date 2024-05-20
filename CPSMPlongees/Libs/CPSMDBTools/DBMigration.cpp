#include "DBMigration.hpp"

namespace cpsm::db::migration {

/*#########################*/
/*#                       #*/
/*#  Migration functions  #*/
/*#                       #*/
/*#########################*/

CPSM_DECLARE_MIGRATION_STEP(0, 0, 1) {
  SPDLOG_INFO("First migration function that should be called only for testing!");
  SPDLOG_INFO("Migrating from {} to {}", from_version, target_version);
  std::ignore = database;
  return true;
}

/*#########################*/
/*#                       #*/
/*#     User functions    #*/
/*#                       #*/
/*#########################*/

std::vector<MigrationStep> GetOrderedMigrationSteps() {
  auto steps{GetMigrationSteps()};
  std::sort(
      steps.begin(), steps.end(), [](const auto& a, const auto& b) { return a.target_version < b.target_version; });
  return steps;
}

bool MigrateDB(updt::Version from_version, updt::Version to_version, QSqlDatabase database) {
  /* Please note that everything running here may take place in a transaction */
  SPDLOG_INFO("[DB_MIGRATION] Migration steps available:");
  const auto kMigrationSteps{GetOrderedMigrationSteps()};
  for (const auto& step : kMigrationSteps) {
    SPDLOG_INFO("[DB_MIGRATION] \tMigrate to <{}>", step.target_version);
  }

  for (const auto& step : kMigrationSteps) {
    if (step.target_version <= from_version) {
      SPDLOG_INFO(
          "[DB_MIGRATION] Ignore migration to <{}> as the target version is older than the start migration version "
          "<{}>",
          step.target_version,
          from_version);
      continue;
    }

    SPDLOG_INFO("[DB_MIGRATION] Migrate from <{}> to <{}>", from_version, step.target_version);
    if (!step.migration_function(from_version, step.target_version, database)) {
      SPDLOG_CRITICAL("[DB_MIGRATION] Migration failed from <{}> to <{}>", from_version, step.target_version);
      return false;
    }
    from_version = step.target_version;
  }

  if (from_version != cpsm::consts::kCurrentVersion) {
    SPDLOG_CRITICAL(
        "The migration was not complete! The target version is <{}> but the last version reached by the migration was "
        "<{}>",
        to_version,
        from_version);
    return false;
  }
  return true;
}

NeedsMigrationState VersionNeedsMigration(updt::Version current_version, updt::Version target_version) {
  /* Version 0,0,0 is considered default -> first start of the software. Normal behavior */
  if (current_version == updt::Version{0, 0, 0}) {
    return NeedsMigrationState::kNoMigrationNeeded;
  }

  if (current_version == target_version) {
    return NeedsMigrationState::kNoMigrationNeeded;
  }

  if (current_version < target_version) {
    return NeedsMigrationState::kUpgradeNeeded;
  }

  return NeedsMigrationState::kDowngradeNeeded;
}

}  // namespace cpsm::db::migration
