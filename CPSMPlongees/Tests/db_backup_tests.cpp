#include <DBSaver.hpp>
#include <DBUtils.hpp>
#include <RawStructs.hpp>

#include <Logger/logger.hpp>
#include <Utils/AsyncFileCopyer.hpp>
#include <gtest/gtest.h>

TEST(CPSMTests, NoBackups) {
  db::ExistingBackupList backups;
  auto result = db::GetBackupsUpdate(backups);
  EXPECT_TRUE(std::get<0>(result).empty());
  EXPECT_TRUE(std::get<1>(result));
}

TEST(CPSMTests, BackupsWithin7Days) {
  QDateTime now = QDateTime::currentDateTime();
  db::ExistingBackupList backups = {{now, "backup0"}, {now.addDays(-1), "backup1"}, {now.addDays(-2), "backup2"}};
  auto result = db::GetBackupsUpdate(backups);
  EXPECT_TRUE(std::get<0>(result).empty());
  EXPECT_FALSE(std::get<1>(result));
}

TEST(CPSMTests, BackupsWithin30Days) {
  QDateTime now = QDateTime::currentDateTime();
  db::ExistingBackupList backups = {
      {now.addDays(-10), "backup1"}, {now.addDays(-20), "backup2"}, {now.addDays(-25), "backup3"}};
  auto result = db::GetBackupsUpdate(backups);
  EXPECT_EQ(std::get<0>(result).size(), 1);
  EXPECT_TRUE(std::get<1>(result));
}

TEST(CPSMTests, BackupsWithin365Days) {
  QDateTime now = QDateTime::currentDateTime();
  db::ExistingBackupList backups = {{now.addDays(-100), "backup1"}, {now.addDays(-200), "backup2"}};
  auto result = db::GetBackupsUpdate(backups);
  EXPECT_EQ(std::get<0>(result).size(), 1);
  EXPECT_TRUE(std::get<1>(result));
}

TEST(CPSMTests, BackupsOlderThan365Days) {
  QDateTime now = QDateTime::currentDateTime();
  db::ExistingBackupList backups = {{now.addDays(-400), "backup1"}, {now.addDays(-500), "backup2"}};
  auto result = db::GetBackupsUpdate(backups);
  EXPECT_EQ(std::get<0>(result).size(), 2);
  EXPECT_TRUE(std::get<1>(result));
}

TEST(CPSMTests, MixedBackups) {
  QDateTime now = QDateTime::currentDateTime();
  db::ExistingBackupList backups = {{now.addDays(0), "backup0"},
                                    {now.addDays(-1), "backup1"},
                                    {now.addDays(-10), "backup2"},
                                    {now.addDays(-100), "backup3"},
                                    {now.addDays(-400), "backup4"}};
  auto result = db::GetBackupsUpdate(backups);
  EXPECT_EQ(std::get<0>(result).size(), 1);
  EXPECT_FALSE(std::get<1>(result));
}
