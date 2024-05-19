#include "DBMigration.hpp"

#include <vector>

#include "CPSMGlobals.hpp"

namespace {

bool MigrateTo1_0_0(const updt::Version& from_version, const updt::Version& target_version, QSqlDatabase database) {
  SPDLOG_INFO("[DB_MIGRATION] Migrate from <{}> to <{}>", from_version, target_version);
  std::ignore = database;

  return true;
}

struct MigrationStep {
  updt::Version target_version;
  std::function<bool(const updt::Version&, const updt::Version&, QSqlDatabase)> migration_function;
};

std::vector<MigrationStep>& GetMutableMigrationSteps() {
  static std::vector<MigrationStep> migration_steps{
      {updt::Version{1, 0, 0}, MigrateTo1_0_0},
  };
  return migration_steps;
}

const std::vector<MigrationStep>& GetMigrationSteps() {
  return GetMutableMigrationSteps();
}

}  // namespace

namespace cpsm::db {

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

bool MigrateDB(updt::Version from_version, updt::Version to_version, QSqlDatabase database) {
  /* Please note that everything running here may take place in a transaction */

  for (const auto& step : GetMigrationSteps()) {
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

}  // namespace cpsm::db
