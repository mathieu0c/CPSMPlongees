#pragma once

#include <DBStatic.hpp>
#include <DBUtils.hpp>
#include <tmp.hpp>

#include "Database.hpp"

namespace cpsm::db {

using DBType = ::db::DBType;

struct MultipleStoreResult {
  enum ErrorCode : int32_t {
    kDefault = 0,
    kOk = 1,
    kFailedToStartTransaction = 2,
    kFailedToStoreElement = 3,
    kFailedToRollback = 4,
    kFailedToCommit = 5
  };

  ErrorCode err_code{};

  bool IsOk() const {
    return err_code == kOk;
  }
  operator bool() const {
    return IsOk();
  }
};

#define DBInfo_VAR_LIST(COLUMN)                               \
  COLUMN(dbinfo_id, DBType::INTEGER, true);                   \
  COLUMN(latest_cpsm_soft_version_used, DBType::TEXT, false); \
  COLUMN(latest_cpsm_db_version_used, DBType::TEXT, false);   \
  COLUMN(latest_open_date, DBType::DATETIME, false);
DB_DECLARE_STRUCT(DBInfo, DBInfo, val.dbinfo_id <= 0, DBInfo_VAR_LIST)

/* Diver */
#define Diver_VAR_LIST(COLUMN)                    \
  COLUMN(diver_id, DBType::INTEGER, true);        \
  COLUMN(first_name, DBType::TEXT, false);        \
  COLUMN(last_name, DBType::TEXT, false);         \
  COLUMN(birth_date, DBType::DATE, false);        \
  COLUMN(email, DBType::TEXT, false);             \
  COLUMN(phone_number, DBType::TEXT, false);      \
  COLUMN(address_id, DBType::INTEGER, false);     \
  COLUMN(license_number, DBType::TEXT, false);    \
  COLUMN(certif_date, DBType::DATE, false);       \
  COLUMN(diver_level_id, DBType::INTEGER, false); \
  COLUMN(registration_date, DBType::DATE, false); \
  COLUMN(member_date, DBType::DATE, false);       \
  COLUMN(paid_dives, DBType::INTEGER, false);     \
  COLUMN(gear_regulator, DBType::INTEGER, false); \
  COLUMN(gear_suit, DBType::INTEGER, false);      \
  COLUMN(gear_computer, DBType::INTEGER, false);  \
  COLUMN(gear_jacket, DBType::INTEGER, false);
DB_DECLARE_STRUCT(Diver, Divers, val.diver_id <= 0, Diver_VAR_LIST)

int GetDiverDiveCount(const Diver& diver);
std::optional<int> GetDiverDiveBalance(const Diver& diver);
bool IsDiverMedicalCertificateValid(const Diver& diver);
bool IsDiverCurrentlyRegistered(const Diver& diver);
bool IsDiverCurrentlyAMember(QDate diver_member_date);
bool IsDiverCurrentlyAMember(const Diver& diver);
int32_t GetDiverAge(const QDate& diver_birth_date);
int32_t GetDiverAge(const Diver& diver);

#define DiverAddress_VAR_LIST(COLUMN)        \
  COLUMN(address_id, DBType::INTEGER, true); \
  COLUMN(address, DBType::TEXT, false);      \
  COLUMN(postal_code, DBType::TEXT, false);  \
  COLUMN(city, DBType::TEXT, false);
DB_DECLARE_STRUCT(DiverAddress, DiversAddresses, val.address_id <= 0, DiverAddress_VAR_LIST)

/* Store a diver and its address... The address gets priority on the address_id of both elements.
 * The address is stored first and the id is then set in the diver before storing it.
 * This allows for new address to be added beforehand. The newly added address's id will then be set in the diver
 */
struct StoreDiverAndAddressResult : public MultipleStoreResult {
  enum DetailErrCode : int32_t { kFailedToStoreAddress = 0, kFailedToStoreDiver = 1 };
  DetailErrCode err_details{};
  Diver stored_diver;
  DiverAddress stored_address;
};
StoreDiverAndAddressResult StoreDiverAndItsAddress(db::Diver diver, const DiverAddress& address);

#define Dive_VAR_LIST(COLUMN)                \
  COLUMN(dive_id, DBType::INTEGER, true);    \
  COLUMN(datetime, DBType::DATETIME, false); \
  COLUMN(diving_site_id, DBType::INTEGER, false);
DB_DECLARE_STRUCT(Dive, Dives, val.dive_id <= 0, Dive_VAR_LIST)

#define DiveMember_VAR_LIST(COLUMN)        \
  COLUMN(dive_id, DBType::INTEGER, true);  \
  COLUMN(diver_id, DBType::INTEGER, true); \
  COLUMN(diving_type_id, DBType::INTEGER, false);
DB_DECLARE_STRUCT(DiveMember, DivesMembers, val.dive_id <= 0 || val.diver_id <= 0, DiveMember_VAR_LIST)

struct StoreDiveAndDiversResult : public MultipleStoreResult {
  enum DetailErrCode : int32_t { kFailedToStoreDive = 0, kFailedToStoreDiveMember = 1 };
  DetailErrCode err_details{};
  Dive stored_dive;
  std::vector<DiveMember> stored_dive_members;
};
StoreDiveAndDiversResult StoreDiveAndItsMembers(Dive dive, const std::vector<DiveMember>& members);

#define DivingSites_VAR_LIST(COLUMN)             \
  COLUMN(diving_site_id, DBType::INTEGER, true); \
  COLUMN(site_name, DBType::TEXT, false);
DB_DECLARE_STRUCT(DivingSite, DivingSites, val.diving_site_id <= 0, DivingSites_VAR_LIST)

#define DivingTypes_VAR_LIST(COLUMN)             \
  COLUMN(diving_type_id, DBType::INTEGER, true); \
  COLUMN(type_name, DBType::TEXT, false);
DB_DECLARE_STRUCT(DivingType, DivingTypes, val.diving_type_id <= 0, DivingTypes_VAR_LIST)

#define DiverLevels_VAR_LIST(COLUMN)             \
  COLUMN(diver_level_id, DBType::INTEGER, true); \
  COLUMN(level_name, DBType::TEXT, false);
DB_DECLARE_STRUCT(DiverLevel, DiversLevels, val.diver_level_id <= 0, DiverLevels_VAR_LIST)

}  // namespace cpsm::db
