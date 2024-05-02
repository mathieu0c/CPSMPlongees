#include <DBUtils.hpp>
#include <RawStructs.hpp>

#include <Logger/logger.hpp>
#include <gtest/gtest.h>

#include "Database.hpp"
#include "test_common.hpp"

QSqlDatabase dbt{QSqlDatabase::database()};

TEST(CPSMTests, DevTest) {
  REINIT_DB;
  //  utils::Chrono<std::chrono::microseconds> req_time0{};
  //  auto results0{
  //      db::readLFromDB<DiveMember>(dbt, db::ExtractDiveMember, "SELECT * FROM %0", {DiveMember::db_table}, {})};
  //  SPDLOG_DEBUG("Got divers in {}µs", req_time0.Time());
  //  for (const auto &e : results0) {
  //    SPDLOG_DEBUG("{}", e);
  //  }
  //  return;

  //  if (!DeleteDiveMember(dbt, test)) {
  //    SPDLOG_ERROR("Failed to delete dive member: {}", test);
  //  }

  //  utils::Chrono<std::chrono::microseconds> req_time{};
  //  auto results{db::readLFromDB<DiveMember>(dbt, db::ExtractDiveMember, "SELECT * FROM %0", {DiveMember::db_table},
  //  {})}; SPDLOG_DEBUG("Got divers in {}µs", req_time.Time()); for (const auto &e : results) {
  //    SPDLOG_DEBUG("{}", e);
  //  }
}

TEST(CPSMTests, DBInitTest) {
  REINIT_DB;
}

TEST(CPSMTests, MultiplePrimaryKeys_DivesMembers) {
  REINIT_DB;

  using db::DiveMember;
  using db::Diver;

  DiveMember test{1, 2, 1};
  SPDLOG_DEBUG("{}", test);
  ASSERT_TRUE(UpdateDiveMember(dbt, test));
  //  ASSERT_TRUE(false);

  auto kTestRead{db::GetDiveMemberFromId(dbt, {1, 2})};
  if (!kTestRead) {
    SPDLOG_WARN("Failed to read DiveMember");
  } else {
    SPDLOG_INFO("Read DiveMember is: {}", *kTestRead);
  }

  ASSERT_TRUE(kTestRead);
}

TEST(CPSMTests, DiveDeletion) {
  REINIT_DB;

  const auto kDiveTarget{2};

  auto count{db::queryCount(dbt, "SELECT * FROM DivesMembers WHERE dive_id = ?", {}, {kDiveTarget})};
  ASSERT_EQ(count, 3);

  ASSERT_TRUE(db::queryDelete(dbt, "DELETE FROM Dives WHERE dive_id = ?;", {}, {kDiveTarget}));
  count = db::queryCount(dbt, "SELECT * FROM DivesMembers WHERE dive_id = ?", {}, {kDiveTarget});
  ASSERT_EQ(count, 0);

  using db::Diver;
  Diver tmp{};
  tmp.diver_id++;
  tmp.email = "Biduile@machin.fr";

  utils::Chrono<std::chrono::microseconds> req_time{};
  auto results{db::readLFromDB<Diver>(dbt, db::ExtractDiver, "SELECT * FROM %0", {Diver::db_table}, {})};
  SPDLOG_DEBUG("Got divers in {}µs", req_time.Time());

  for (const auto& e : results) {
    SPDLOG_DEBUG("{}", e);
  }
}

TEST(CPSMTests, DiverAddress) {
  REINIT_DB;

  auto lambda_get_address_count{[]() {
    std::map<int32_t, int32_t> tmp;
    db::readLFromDB<int>(dbt,
                         [&tmp](const QSqlQuery& query) {
                           const int32_t kAddressId{query.value(0).toInt()};
                           const int32_t kAddressIdCount{query.value(1).toInt()};

                           tmp[kAddressId] = kAddressIdCount;

                           return 0;
                         },
                         "SELECT address_id, count(address_id) FROM Divers group by address_id",
                         {},
                         {});
    return tmp;
  }};
  const int32_t kTargetAddressId{2};
  const std::array kTargetDiverIds{2, 5};

  auto lambda_get_target_address_count{[]() {
    return db::queryCount(
        dbt, "SELECT address_id FROM %0 WHERE address_id = ?;", {db::DiverAddress::db_table}, {kTargetAddressId});
  }};

  const auto kDiverAddressCountBefore{lambda_get_address_count()};
  ASSERT_TRUE(kDiverAddressCountBefore.find(kTargetAddressId) != kDiverAddressCountBefore.end());

  EXPECT_EQ(kDiverAddressCountBefore.at(kTargetAddressId), 2);
  EXPECT_EQ(lambda_get_target_address_count(), 1);

  EXPECT_TRUE(
      db::queryDelete(dbt, "DELETE FROM %0 WHERE diver_id = ?;", {db::Diver::db_table}, {kTargetDiverIds.back()}));
  const auto kDiverAddressCountAfter{lambda_get_address_count()};
  EXPECT_EQ(kDiverAddressCountAfter.at(kTargetAddressId), 1);
  EXPECT_EQ(lambda_get_target_address_count(), 1);

  EXPECT_TRUE(
      db::queryDelete(dbt, "DELETE FROM %0 WHERE diver_id = ?;", {db::Diver::db_table}, {kTargetDiverIds.front()}));
  const auto kDiverAddressCountAfter2{lambda_get_address_count()};
  EXPECT_TRUE(kDiverAddressCountAfter2.find(kTargetAddressId) == kDiverAddressCountAfter2.end());
  EXPECT_EQ(lambda_get_target_address_count(), 0);

  // ---------

  const auto kDiverList{
      db::readLFromDB<db::Diver>(dbt, db::ExtractDiver, "SELECT * FROM %0", {db::Diver::db_table}, {})};
  for (const auto& e : kDiverList) {
    SPDLOG_INFO("{}", e);
  }

  SPDLOG_INFO("-----------");

  const auto kAddressList{db::readLFromDB<db::DiverAddress>(
      dbt, db::ExtractDiverAddress, "SELECT * FROM %0", {db::DiverAddress::db_table}, {})};
  for (const auto& e : kAddressList) {
    SPDLOG_INFO("{}", e);
  }
}
