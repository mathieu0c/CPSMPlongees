#pragma once

#include <CPSMGlobals.hpp>
#include <DBFormatting.hpp>
#include <DBUtils.hpp>

namespace cpsm::db {

template <bool kDeleteOnCreation = false, bool kFillDBWithTestsValues = false>
bool InitDB(const auto& kFileName) {
  SPDLOG_INFO("--------- DB init function ---------");

  SPDLOG_INFO("Using db file: {}", QFileInfo{kFileName}.absoluteFilePath());

#ifndef CMAKE_DEBUG_MODE
  if constexpr (kDeleteOnCreation && !consts::kIsBuiltAsMockup) {
    SPDLOG_WARN("We don't want to delete the database when we are not in debug!!!!");
    throw std::runtime_error{"Trying to delete DB in non-debug mode"};
  }
#endif

  if constexpr (kDeleteOnCreation && consts::kIsBuiltAsMockup) {
#ifndef CMAKE_DEBUG_MODE
#warning "Requesting to delete db file not in DEBUG mode"
#endif
    SPDLOG_WARN("Deleting DB file: {}: success? {}", kFileName, QFile::remove(kFileName));
  }

  if (!::db::OpenLocal(kFileName)) {
    SPDLOG_ERROR("Failed to open db: {}", kFileName);
    return false;
  }

  QSqlDatabase db{QSqlDatabase::database()};

  if (!::db::ExecScript(":/DBScripts/CreateDB.sql", &db)) {
    SPDLOG_ERROR("Failed to execute DB creation script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed DB creation script");
  }

  if (!::db::ExecScript(":/DBScripts/SetupTriggers.sql", &db)) {
    SPDLOG_ERROR("Failed to setup DB triggers creation script");
    return false;
  } else {
    SPDLOG_INFO("Successfully executed DB triggers creation script");
  }

  if constexpr (kFillDBWithTestsValues && consts::kIsBuiltAsMockup) {
#ifndef CMAKE_DEBUG_MODE
#warning Requesting to fill db with tests values not in DEBUG mode
#endif
    if (!::db::ExecScript(":/DBScripts/FillTestDB.sql", &db)) {
      SPDLOG_ERROR("Failed to execute fill test DB script");
      return false;
    } else {
      SPDLOG_INFO("Successfully executed fill test DB script");
    }
  }

  SPDLOG_INFO("--------- DB init function END ---------");
  return true;
}

}  // namespace cpsm::db
