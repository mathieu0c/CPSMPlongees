#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <QDirIterator>

#include <Logger/logger_setup.hpp>
#include <gtest/gtest.h>

#include "test_common.hpp"

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(DBScripts);

  ::testing::InitGoogleTest(&argc, argv);

  logger::SetupLoggerRotating("TEST_LOGS.log", 1);

  if (!test::OpenDB()) {
    SPDLOG_CRITICAL("Could not open test database. Abort tests");
    return -1;
  }

  SPDLOG_INFO("Listing qrc files");
  QDirIterator it(":", QDirIterator::Subdirectories);
  while (it.hasNext()) {
    SPDLOG_INFO("QRC: {}", it.next());
  }

  return RUN_ALL_TESTS();
}
