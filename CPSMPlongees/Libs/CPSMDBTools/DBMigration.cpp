#include "DBMigration.hpp"

#include <Database.hpp>

namespace cpsm::db::migration {

/*#########################*/
/*#                       #*/
/*#  Migration functions  #*/
/*#                       #*/
/*#########################*/

CPSM_DECLARE_MIGRATION_STEP(0, 0, 1) {
  SPDLOG_INFO(
      "First migration function that should be called only for testing!");
  SPDLOG_INFO("Migrating from {} to {}", from_version, target_version);
  std::ignore = database;
  return true;
}

CPSM_DECLARE_MIGRATION_STEP(1, 1, 0) {
  SPDLOG_INFO("[DB_MIGRATION] v1.1.0 - Adding NitroxDiversLevels table and "
              "migrating Divers table");
  std::ignore = from_version;
  std::ignore = target_version;

  const auto kExec{[&](const QString &sql) -> bool {
    auto q{::db::ExecQuery(database, sql, {}, {})};
    if (!q) {
      SPDLOG_CRITICAL("[DB_MIGRATION] v1.0.0 - Query failed: {}", sql);
      return false;
    }
    return true;
  }};

  // Create NitroxDiversLevels table
  if (!kExec(R"(CREATE TABLE IF NOT EXISTS "NitroxDiversLevels" (
    nitrox_diver_level_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    level_name TEXT UNIQUE
  ))"))
    return false;

  // Seed NitroxDiversLevels
  if (!kExec(
          R"(INSERT OR IGNORE INTO NitroxDiversLevels (nitrox_diver_level_id, level_name) VALUES
    (1,'VIDE'),
    (2,'BASE'),
    (3,'CONFIRMÉ'),
    (4,'MONITEUR')
  )"))
    return false;

  // Rebuild Divers with nitrox_diver_level_id FK (SQLite requires full table
  // rebuild to add FK constraints)
  if (!kExec(R"(CREATE TABLE Divers_new (
    diver_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    first_name TEXT,
    last_name TEXT,
    birth_date TEXT,
    email TEXT,
    phone_number TEXT,
    address_id INTEGER,
    license_number TEXT,
    certif_date TEXT,
    diver_level_id INTEGER,
    nitrox_diver_level_id INTEGER,
    first_registration_date TEXT,
    registration_date TEXT,
    member_date TEXT,
    paid_dives INTEGER,
    gear_regulator INTEGER DEFAULT (0),
    gear_suit INTEGER DEFAULT (0),
    gear_computer INTEGER DEFAULT (0),
    gear_jacket INTEGER DEFAULT (0),
    CONSTRAINT Divers_DiverLevels_FK FOREIGN KEY (diver_level_id) REFERENCES "DiversLevels"(diver_level_id) ON DELETE SET DEFAULT ON UPDATE SET DEFAULT,
    CONSTRAINT Divers_NitroxDiversLevels_FK FOREIGN KEY (nitrox_diver_level_id) REFERENCES "NitroxDiversLevels"(nitrox_diver_level_id) ON DELETE SET DEFAULT ON UPDATE SET DEFAULT,
    CONSTRAINT Divers_DiversAddresses_FK FOREIGN KEY (address_id) REFERENCES DiversAddresses(address_id) ON DELETE SET NULL
  ))"))
    return false;

  if (!kExec(R"(INSERT INTO Divers_new
    (diver_id,first_name,last_name,birth_date,email,phone_number,address_id,license_number,
     certif_date,diver_level_id,nitrox_diver_level_id,first_registration_date,registration_date,
     member_date,paid_dives,gear_regulator,gear_suit,gear_computer,gear_jacket)
    SELECT diver_id,first_name,last_name,birth_date,email,phone_number,address_id,license_number,
     certif_date,diver_level_id,1,first_registration_date,registration_date,
     member_date,paid_dives,gear_regulator,gear_suit,gear_computer,gear_jacket
    FROM Divers
  )"))
    return false;

  if (!kExec("DROP TABLE Divers"))
    return false;
  if (!kExec("ALTER TABLE Divers_new RENAME TO Divers"))
    return false;

  return true;
}

/*#########################*/
/*#                       #*/
/*#     User functions    #*/
/*#                       #*/
/*#########################*/

std::vector<MigrationStep> GetOrderedMigrationSteps() {
  auto steps{GetMigrationSteps()};
  std::sort(steps.begin(), steps.end(), [](const auto &a, const auto &b) {
    return a.target_version < b.target_version;
  });
  return steps;
}

bool MigrateDB(updt::Version from_version, updt::Version to_version,
               QSqlDatabase database) {
  /* Please note that everything running here may take place in a transaction */
  SPDLOG_INFO("[DB_MIGRATION] Migration steps available:");
  const auto kMigrationSteps{GetOrderedMigrationSteps()};
  for (const auto &step : kMigrationSteps) {
    SPDLOG_INFO("[DB_MIGRATION] \tMigrate to <{}>", step.target_version);
  }

  for (const auto &step : kMigrationSteps) {
    if (step.target_version <= from_version) {
      SPDLOG_INFO("[DB_MIGRATION] Ignore migration to <{}> as the target "
                  "version is older than the start migration version "
                  "<{}>",
                  step.target_version, from_version);
      continue;
    }

    SPDLOG_INFO("[DB_MIGRATION] Migrate from <{}> to <{}>", from_version,
                step.target_version);
    if (!step.migration_function(from_version, step.target_version, database)) {
      SPDLOG_CRITICAL("[DB_MIGRATION] Migration failed from <{}> to <{}>",
                      from_version, step.target_version);
      return false;
    }
    from_version = step.target_version;
  }

  if (from_version != to_version) {
    SPDLOG_WARN("[DB_MIGRATION] No migration step reached the target version "
                "<{}>. Last version reached: <{}>. Assuming no schema changes "
                "were needed.",
                to_version, from_version);
  }
  return true;
}

NeedsMigrationState VersionNeedsMigration(updt::Version current_version,
                                          updt::Version target_version) {
  /* Version 0,0,0 is considered default -> first start of the software. Normal
   * behavior */
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

} // namespace cpsm::db::migration
