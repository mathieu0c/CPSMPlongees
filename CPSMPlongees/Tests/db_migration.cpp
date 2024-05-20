#include <DBMigration.hpp>
#include <DBUtils.hpp>
#include <RawStructs.hpp>

#include <Logger/logger.hpp>
#include <gtest/gtest.h>

#include "test_common.hpp"

namespace cpsm::db::migration {
class MigrationStepTo0_0_Min1 {
  static bool MigrationFunction(const updt::Version& from_version, const updt::Version& target_version,
                                QSqlDatabase database);
  static const MigrationStep& kMigrationStep;
};
const MigrationStep& MigrationStepTo0_0_Min1::kMigrationStep{
    RegisterMigrationStep({updt::Version{0, 0, -1}, MigrationStepTo0_0_Min1::MigrationFunction})};
bool MigrationStepTo0_0_Min1::MigrationFunction(const updt::Version& from_version, const updt::Version& target_version,
                                                QSqlDatabase database) {
  SPDLOG_INFO("Used to test fail of db migration from <{}> to <{}> (needs to run a migration from version 0,0,-2",
              from_version,
              target_version);
  std::ignore = database;
  return false;
}

CPSM_DECLARE_MIGRATION_STEP(0, 0, 0) {
  SPDLOG_INFO("Used to test fail db migration with multiple successful steps", from_version, target_version);
  std::ignore = database;
  return true;
}
}  // namespace cpsm::db::migration

TEST(CPSMTests, DatabaseMigration) {
  REINIT_DB;

  auto database{DBT};

  const auto kDBInfoOpt{cpsm::db::GetDBInfoFromId(database, {1})};
  ASSERT_TRUE(kDBInfoOpt);

  auto db_info{*kDBInfoOpt};
  SPDLOG_INFO("");
  SPDLOG_INFO("DB infos read: {}", db_info);

  const auto kLastDBSoftVersionOpt{updt::Version::FromString(db_info.latest_cpsm_soft_version_used)};
  ASSERT_TRUE(kLastDBSoftVersionOpt);
  const auto kLastDBSoftVersion{*kLastDBSoftVersionOpt};

  /* -- DB actual Migration -- */

  const auto kTransactionStart{database.transaction()};
  if (!kTransactionStart) {
    SPDLOG_CRITICAL("Could not start transaction: <{}>", database.lastError().text());
  }
  ASSERT_TRUE(kTransactionStart);

  const auto kNeedsMigration{
      cpsm::db::migration::VersionNeedsMigration(kLastDBSoftVersion, cpsm::consts::kCurrentVersion)};
  switch (kNeedsMigration) {
    case cpsm::db::migration::NeedsMigrationState::kNoMigrationNeeded:
      break;
    case cpsm::db::migration::NeedsMigrationState::kUpgradeNeeded: /* We'll check that later for lisibility reason */
      break;
    case cpsm::db::migration::NeedsMigrationState::kDowngradeNeeded: {
      SPDLOG_CRITICAL("What the hell. We should not end up here");
      break;
    }
    default:
      SPDLOG_CRITICAL("Unknown migration need type: {}", static_cast<int>(kNeedsMigration));
      break;
  }
  EXPECT_EQ(kNeedsMigration, cpsm::db::migration::NeedsMigrationState::kNoMigrationNeeded);

  /* We have to force it as we inhibit the migration from the default db version (only on creation) */
  if (kNeedsMigration == cpsm::db::migration::NeedsMigrationState::kUpgradeNeeded || true) {
    const auto kUpgradeSuccess{
        cpsm::db::migration::MigrateDB(kLastDBSoftVersion, cpsm::consts::kCurrentVersion, database)};
    if (!kUpgradeSuccess) {
      /* Make sure we rollback to prevent problems on next opening */
      ASSERT_TRUE(database.rollback());
    }

    ASSERT_TRUE(kUpgradeSuccess); /* We ended up failing at some point */
  }

  db_info.latest_cpsm_soft_version_used = to_string(cpsm::consts::kCurrentVersion);
  db_info.latest_cpsm_db_version_used = to_string(cpsm::consts::kCurrentVersion);
  db_info.latest_open_date = QDateTime::currentDateTime();
  SPDLOG_INFO("Updating DB infos to {}", db_info);
  const auto kUpdateInfoSuccess{cpsm::db::UpdateDBInfo(database, db_info)};
  SPDLOG_INFO("Update db info success? {}", kUpdateInfoSuccess.has_value());
  if (!kUpdateInfoSuccess) {
    ASSERT_TRUE(database.rollback());
  }
  ASSERT_TRUE(database.commit());
  SPDLOG_INFO("");
}

TEST(CPSMTests, DatabaseMigrationFail) {
  REINIT_DB;

  auto dbt{DBT};

  EXPECT_FALSE(cpsm::db::migration::MigrateDB(updt::Version{0, 0, -2}, cpsm::consts::kCurrentVersion, dbt));
}

TEST(CPSMTests, MultiMigration) {
  REINIT_DB;

  auto dbt{DBT};

  EXPECT_TRUE(cpsm::db::migration::MigrateDB(updt::Version{0, 0, -1}, updt::Version{0, 0, 1}, dbt));
}
