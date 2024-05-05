#pragma once

#include <DBStatic.hpp>
#include <DBUtils.hpp>
#include <tmp.hpp>

#include "Database.hpp"

namespace db {

/* Diver */
#define Diver_VAR_LIST(COLUMN)            \
  COLUMN(diver_id, INTEGER, true);        \
  COLUMN(first_name, TEXT, false);        \
  COLUMN(last_name, TEXT, false);         \
  COLUMN(birth_date, DATE, false);        \
  COLUMN(email, TEXT, false);             \
  COLUMN(phone_number, TEXT, false);      \
  COLUMN(address_id, INTEGER, false);     \
  COLUMN(license_number, TEXT, false);    \
  COLUMN(certif_date, DATE, false);       \
  COLUMN(diver_level_id, INTEGER, false); \
  COLUMN(registration_date, DATE, false); \
  COLUMN(is_member, INTEGER, false);      \
  COLUMN(member_date, DATE, false);       \
  COLUMN(paid_dives, INTEGER, false);     \
  COLUMN(gear_regulator, INTEGER, false); \
  COLUMN(gear_suit, INTEGER, false);      \
  COLUMN(gear_computer, INTEGER, false);  \
  COLUMN(gear_jacket, INTEGER, false);
DB_DECLARE_STRUCT(Diver, Divers, val.diver_id <= 0, Diver_VAR_LIST)
int GetDiverDiveCount(const Diver& diver);

#define DiverAddress_VAR_LIST(COLUMN) \
  COLUMN(address_id, INTEGER, true);  \
  COLUMN(address, TEXT, false);       \
  COLUMN(postal_code, TEXT, false);   \
  COLUMN(city, TEXT, false);
DB_DECLARE_STRUCT(DiverAddress, DiversAddresses, val.address_id <= 0, DiverAddress_VAR_LIST)

/* Store a diver and its address... The address gets priority on the address_id of both elements.
 * The address is stored first and the id is then set in the diver before storing it.
 * This allows for new address to be added beforehand. The newly added address's id will then be set in the diver
 */
struct StoreDiverAndAddressResult {
  enum ErrCode : int32_t {
    kDefault = 0,
    kOk = 1,
    kFailedToStartTransaction = 2,
    kFailedToStoreAddress = 3,
    kFailedToStoreDiver = 4,
    kFailedToRollback = 5,
    kFailedToCommit = 6
  };
  Diver stored_diver;
  DiverAddress stored_address;
  ErrCode err_code;

  bool IsOk() const {
    return err_code == kOk;
  }
  operator bool() const {
    return IsOk();
  }
};
StoreDiverAndAddressResult StoreDiverAndItsAddress(db::Diver diver, const DiverAddress& address);

#define Dive_VAR_LIST(COLUMN)        \
  COLUMN(dive_id, INTEGER, true);    \
  COLUMN(datetime, DATETIME, false); \
  COLUMN(diving_site_id, INTEGER, false);
DB_DECLARE_STRUCT(Dive, Dives, val.dive_id <= 0, Dive_VAR_LIST)

#define DiveMember_VAR_LIST(COLUMN) \
  COLUMN(dive_id, INTEGER, true);   \
  COLUMN(diver_id, INTEGER, true);  \
  COLUMN(diving_type_id, INTEGER, false);
DB_DECLARE_STRUCT(DiveMember, DivesMembers, val.dive_id <= 0 || val.diver_id <= 0, DiveMember_VAR_LIST)

#define DivingSites_VAR_LIST(COLUMN)     \
  COLUMN(diving_site_id, INTEGER, true); \
  COLUMN(site_text, TEXT, false);
DB_DECLARE_STRUCT(DivingSite, DivingSites, val.diving_site_id <= 0, DivingSites_VAR_LIST)

#define DivingTypes_VAR_LIST(COLUMN)     \
  COLUMN(diving_type_id, INTEGER, true); \
  COLUMN(type_name, TEXT, false);
DB_DECLARE_STRUCT(DivingType, DivingTypes, val.diving_type_id <= 0, DivingTypes_VAR_LIST)

#define DiverLevels_VAR_LIST(COLUMN)     \
  COLUMN(diver_level_id, INTEGER, true); \
  COLUMN(level_name, TEXT, false);
DB_DECLARE_STRUCT(DiverLevel, DiversLevels, val.diver_level_id <= 0, DiverLevels_VAR_LIST)
}  // namespace db
