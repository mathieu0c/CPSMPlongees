#include "DBMigration.hpp"

#include <vector>

namespace {

std::optional<updt::Version> MigrateTo1_0_0(const updt::Version& from_version) {
  const updt::Version kTargetVersion{1, 0, 0};
  SPDLOG_INFO("[DB_MIGRATION] Migrate from <{}> to <{}>", from_version, kTargetVersion);

  return kTargetVersion;
}

}  // namespace

namespace cpsm::db {

bool MigrateDB(updt::Version from_version) {
  // if (from_version != consts::kCurrentVersion) {
  //   SPDLOG_CRITICAL(
  //       "The migration was not complete! The current version is <{}> but the last version reached by the migration
  //       was "
  //       "<{}>",
  //       consts::kCurrentVersion,
  //       from_version);
  //   return false;
  // }
  return true;
}

}  // namespace cpsm::db
