#pragma once

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <QFileInfo>

#include <gtest/gtest.h>

#define REINIT_DB ASSERT_TRUE(test::ReinitDB())

extern QSqlDatabase dbt;

namespace test {

constexpr bool kDebugTest{false};

inline QSqlDatabase* GetDB() {
  static auto db{QSqlDatabase::database()};

  static bool is_db_init{false};
  if (!is_db_init) {
    QFileInfo db_file{"test_db.sqlite"};
    if (!::db::OpenLocal(db_file.absoluteFilePath())) {
      throw std::runtime_error("Could not create db");
    }
    SPDLOG_INFO("Opened db: {}", db_file);
    is_db_init = true;
  }

  return &db;
}

inline bool FillDBWithTests() {
  if (!::db::ExecScript<false, kDebugTest>(":/DBScripts/FillTestDB.sql", GetDB())) {
    SPDLOG_ERROR("Failed to execute fill test DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed fill test DB script");
  }
  return true;
}

inline bool ClearDB() {
  if (!::db::ExecScript<false, kDebugTest>(":/Rc/ClearDB.sql", GetDB())) {
    SPDLOG_ERROR("Failed to execute clear test DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed clear test DB script");
  }
  return true;
}

inline bool CreateDB() {
  if (!::db::ExecScript<false, kDebugTest>(":/DBScripts/CreateDB.sql", GetDB())) {
    SPDLOG_ERROR("Failed to execute create DB script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed create DB script");
  }
  if (!::db::ExecScript<false, kDebugTest>(":/DBScripts/SetupTriggers.sql", GetDB())) {
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
