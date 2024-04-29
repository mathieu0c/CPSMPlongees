#pragma once

#include <DBStatic.hpp>
#include <DBUtils.hpp>

#include "Database.hpp"

namespace db {

/* Diver */
#define Diver_VAR_LIST(COLUMN)            \
  COLUMN(diver_id, INTEGER, true);        \
  COLUMN(first_name, TEXT, false);        \
  COLUMN(last_name, TEXT, false);         \
  COLUMN(birth_date, TEXT, false);        \
  COLUMN(email, TEXT, false);             \
  COLUMN(phone_number, TEXT, false);      \
  COLUMN(address_id, INTEGER, false);     \
  COLUMN(license_number, TEXT, false);    \
  COLUMN(certif_date, TEXT, false);       \
  COLUMN(diver_level_id, INTEGER, false); \
  COLUMN(is_member, INTEGER, false);      \
  COLUMN(registration_date, TEXT, false); \
  COLUMN(paid_dives, INTEGER, false);     \
  COLUMN(gear_regulator, INTEGER, false); \
  COLUMN(gear_suit, INTEGER, false);      \
  COLUMN(gear_computer, INTEGER, false);  \
  COLUMN(gear_jacket, INTEGER, false);
DB_DECLARE_STRUCT(Diver, Divers, val.diver_id <= 0, Diver_VAR_LIST)

#define DiverAddress_VAR_LIST(COLUMN) \
  COLUMN(address_id, INTEGER, true);  \
  COLUMN(address, TEXT, false);       \
  COLUMN(postal_code, TEXT, false);   \
  COLUMN(city, TEXT, false);
DB_DECLARE_STRUCT(DiverAddress, DiversAddresses, val.address_id <= 0, DiverAddress_VAR_LIST)

struct Dive {
  DB_ELEM(dive_id, INTEGER, true);
  DB_ELEM(datetime, TEXT, false);
  DB_ELEM(diving_site_id, INTEGER, false);
};

#define DiveMember_VAR_LIST(COLUMN) \
  COLUMN(dive_id, INTEGER, true);   \
  COLUMN(diver_id, INTEGER, true);  \
  COLUMN(diving_type_id, INTEGER, false);
DB_DECLARE_STRUCT(DiveMember, DivesMembers, val.dive_id <= 0 || val.diver_id <= 0, DiveMember_VAR_LIST)

struct DivingSite {
  DB_ELEM(diving_site_id, INTEGER, true);
  DB_ELEM(site_text, TEXT, false);
};

struct DivingType {
  DB_ELEM(diving_type_id, INTEGER, true);
  DB_ELEM(type_name, TEXT, false);
};

struct DiverLevel {
  DB_ELEM(diver_level_id, INTEGER, true);
  DB_ELEM(level_name, TEXT, false);
};
}  // namespace db
