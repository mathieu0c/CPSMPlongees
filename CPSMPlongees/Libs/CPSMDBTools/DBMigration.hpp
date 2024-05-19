#pragma once

#include <UpdtGlobal.hpp>

namespace cpsm::db {

bool MigrateDB(updt::Version from_version, updt::Version to_version);

}  // namespace cpsm::db
