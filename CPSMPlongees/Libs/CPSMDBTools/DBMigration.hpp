#pragma once

#include <QSqlDatabase>
#include <UpdtGlobal.hpp>
#include <cstdint>

namespace cpsm::db {

enum class NeedsMigrationState : int32_t { kNoMigrationNeeded, kUpgradeNeeded, kDowngradeNeeded };

NeedsMigrationState VersionNeedsMigration(updt::Version current_version, updt::Version target_version);

bool MigrateDB(updt::Version from_version, updt::Version to_version, QSqlDatabase database);

}  // namespace cpsm::db
