#include <DBUtils.hpp>
#include <RawStructs.hpp>

#include <Logger/logger.hpp>
#include <gtest/gtest.h>

#include "Database.hpp"
#include "test_common.hpp"

TEST(CPSMTests, DBInitTest) {
  REINIT_DB;
  SPDLOG_INFO("Using sql driver: <{}> open?<{}>", DBT.driverName(), DBT.driver()->isOpen());
}

TEST(CPSMTests, MultiplePrimaryKeys_DivesMembers) {
  REINIT_DB;

  using cpsm::db::DiveMember;
  using cpsm::db::Diver;

  DiveMember test{1, 2, 1};
  SPDLOG_DEBUG("{}", test);
  ASSERT_TRUE(UpdateDiveMember(DBT, test));
  //  ASSERT_TRUE(false);

  auto kTestRead{cpsm::db::GetDiveMemberFromId(DBT, {1, 2})};
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
  auto dbt{DBT};

  auto count{db::queryCount(dbt, "SELECT * FROM DivesMembers WHERE dive_id = ?", {}, {kDiveTarget})};
  ASSERT_EQ(count, 3);

  ASSERT_TRUE(db::queryDelete(dbt, "DELETE FROM Dives WHERE dive_id = ?;", {}, {kDiveTarget}));
  count = db::queryCount(dbt, "SELECT * FROM DivesMembers WHERE dive_id = ?", {}, {kDiveTarget});
  ASSERT_EQ(count, 0);

  using cpsm::db::Diver;
  Diver tmp{};
  tmp.diver_id++;
  tmp.email = "Biduile@machin.fr";

  utils::Chrono<std::chrono::microseconds> req_time{};
  auto results{db::readLFromDB<Diver>(DBT, cpsm::db::ExtractDiver, "SELECT * FROM %0", {Diver::db_table}, {})};
  SPDLOG_DEBUG("Got divers in {}µs", req_time.Time());

  for (const auto& e : results) {
    SPDLOG_DEBUG("{}", e);
  }
}

TEST(CPSMTests, DiverAddress) {
  REINIT_DB;

  auto dbt{DBT};

  auto lambda_get_address_count{[]() {
    std::map<int32_t, int32_t> tmp;
    db::readLFromDB<int>(DBT,
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
  auto lambda_get_address_count_in_address_table{[]() {
    auto dbt{DBT};
    return db::queryCount(dbt, "SELECT address_id FROM %0;", {cpsm::db::DiverAddress::db_table}, {});
  }};

  const int32_t kTargetAddressId{2};
  const std::array kTargetDiverIds{2, 5};

  auto lambda_get_target_address_count{[]() {
    auto dbt{DBT};
    return db::queryCount(
        dbt, "SELECT address_id FROM %0 WHERE address_id = ?;", {cpsm::db::DiverAddress::db_table}, {kTargetAddressId});
  }};

  const auto kDiverAddressCountBefore{lambda_get_address_count()};
  ASSERT_TRUE(kDiverAddressCountBefore.find(kTargetAddressId) != kDiverAddressCountBefore.end());

  EXPECT_EQ(kDiverAddressCountBefore.at(kTargetAddressId), 2);
  EXPECT_EQ(lambda_get_target_address_count(), 1);

  EXPECT_TRUE(db::queryDelete(
      dbt, "DELETE FROM %0 WHERE diver_id = ?;", {cpsm::db::Diver::db_table}, {kTargetDiverIds.back()}));
  const auto kDiverAddressCountAfter{lambda_get_address_count()};
  EXPECT_EQ(kDiverAddressCountAfter.at(kTargetAddressId), 1);
  EXPECT_EQ(lambda_get_target_address_count(), 1);

  EXPECT_TRUE(db::queryDelete(
      dbt, "DELETE FROM %0 WHERE diver_id = ?;", {cpsm::db::Diver::db_table}, {kTargetDiverIds.front()}));
  const auto kDiverAddressCountAfter2{lambda_get_address_count()};
  EXPECT_TRUE(kDiverAddressCountAfter2.find(kTargetAddressId) == kDiverAddressCountAfter2.end());
  EXPECT_EQ(lambda_get_target_address_count(), 0);

  SPDLOG_DEBUG("\n\n#############################");

  cpsm::db::DiverAddress new_address{};
  new_address.address = "TEST_ADDR";
  new_address.city = "Lorient";
  new_address.postal_code = "9999";
  const auto kCountBefore{lambda_get_address_count_in_address_table()};
  auto val_opt{cpsm::db::UpdateDiverAddress(DBT, new_address)};
  SPDLOG_DEBUG("Extracted: {}", *val_opt);

  EXPECT_EQ(lambda_get_address_count_in_address_table(), kCountBefore + 1);
  EXPECT_EQ(val_opt->address_id, 6);

  SPDLOG_DEBUG("#############################\n\n");

  // ---------

  const auto kDiverList{db::readLFromDB<cpsm::db::Diver>(
      DBT, cpsm::db::ExtractDiver, "SELECT * FROM %0", {cpsm::db::Diver::db_table}, {})};
  for (const auto& e : kDiverList) {
    SPDLOG_INFO("{}", e);
  }

  SPDLOG_INFO("-----------");

  const auto kAddressList{db::readLFromDB<cpsm::db::DiverAddress>(
      DBT, cpsm::db::ExtractDiverAddress, "SELECT * FROM %0", {cpsm::db::DiverAddress::db_table}, {})};
  for (const auto& e : kAddressList) {
    SPDLOG_INFO("{}", e);
  }
}

TEST(CPSMTests, DiverAndItsAddress) {
  REINIT_DB;

  const auto kOriginalDiverOpt{cpsm::db::GetDiverFromId(DBT, {1})};
  ASSERT_TRUE(kOriginalDiverOpt);

  const auto kOriginalDiver{kOriginalDiverOpt.value()};
  auto diver{kOriginalDiver};

  const auto kOriginalAddressOpt{cpsm::db::GetDiverAddressFromId(DBT, {diver.address_id})};
  ASSERT_TRUE(kOriginalAddressOpt);

  const auto kOriginalAddress{kOriginalAddressOpt.value()};
  auto address{kOriginalAddress};
  address.address_id = {};
  address.address += "_MODIFIED";

  const auto kStoreResult{cpsm::db::StoreDiverAndItsAddress(diver, address)};
  ASSERT_TRUE(kStoreResult);

  const auto& kStoredDiver{kStoreResult.stored_diver};
  const auto& kStoredAddress{kStoreResult.stored_address};
  SPDLOG_DEBUG("\nStored diver: {}\nStored diver address:{}", kStoredDiver, kStoredAddress);

  EXPECT_NE(kStoredAddress.address_id, address.address_id); /* An id should have been affected */
  EXPECT_EQ(kStoredDiver.address_id, kStoredAddress.address_id);
  diver.address_id = kStoredAddress.address_id; /* Appart from this, both should be equals */
  EXPECT_EQ(diver, kStoredDiver);
}
