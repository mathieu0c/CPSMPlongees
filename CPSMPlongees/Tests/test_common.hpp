#pragma once

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <QFileInfo>

#include <gtest/gtest.h>

#define REINIT_DB ASSERT_TRUE(test::ReinitDB())

#define DBT QSqlDatabase::database()

namespace test {

constexpr bool kDebugTest{false};

inline bool OpenDB() {
  auto db{QSqlDatabase::database()};

  QFileInfo db_file{"test_db.sqlite"};
  if (!::db::OpenLocal(db_file.absoluteFilePath())) {
    throw std::runtime_error("Could not create db");
    return false;
  }
  SPDLOG_INFO("Opened db: {}   driver_name=<{}>", db_file, db.driverName());

  return true;
}

inline bool FillDBWithTests() {
  auto dbt{DBT};
  if (!::db::ExecScript<false, kDebugTest>(":/DBScripts/FillTestDB.sql", &dbt)) {
    SPDLOG_ERROR("Failed to execute fill test DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed fill test DB script");
  }
  return true;
}

inline bool ClearDB() {
  auto dbt{DBT};
  if (!::db::ExecScript<false, kDebugTest>(":/Rc/ClearDB.sql", &dbt)) {
    SPDLOG_ERROR("Failed to execute clear test DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed clear test DB script");
  }
  return true;
}

inline bool CreateDB() {
  auto dbt{DBT};
  if (!::db::ExecScript<false, kDebugTest>(":/DBScripts/CreateDB.sql", &dbt)) {
    SPDLOG_ERROR("Failed to execute create DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed create DB script");
  }
  if (!::db::ExecScript<false, kDebugTest>(":/DBScripts/SetupTriggers.sql", &dbt)) {
    SPDLOG_ERROR("Failed to execute setup triggers DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed setup triggers DB script");
  }
  return true;
}

inline bool ReinitDB() {
  return ClearDB() && CreateDB() && FillDBWithTests();
}

}  // namespace test
