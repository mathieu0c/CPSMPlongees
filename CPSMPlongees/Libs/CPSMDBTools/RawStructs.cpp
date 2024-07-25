#include "RawStructs.hpp"

#include <QJsonDocument>

#define ROLLBACK_ON_FAIL_STORE_DIVE_AND_DIVERS(condition, error_code, error_details)                  \
  if (condition) {                                                                                    \
    SPDLOG_WARN("Rollback because of failed condition <{}>\n\tERR=<{}>\n\tDETAILS=<{}>\n\tfrom <{}>", \
                #condition,                                                                           \
                #error_code,                                                                          \
                #error_details,                                                                       \
                __PRETTY_FUNCTION__);                                                                 \
    if (!database.rollback()) {                                                                       \
      out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToRollback;                          \
      return out;                                                                                     \
    } /* else */                                                                                      \
    out.err_code = error_code;                                                                        \
    out.err_details = error_details;                                                                  \
    return out;                                                                                       \
  }

namespace cpsm::db {

int GetDiverDiveCount(const Diver& diver) {
  auto db{::db::Def()};
  return ::db::queryCount(
      db, "SELECT diver_id FROM %0 WHERE %1 = ?", {DiveMember::db_table, DiveMember::diver_id_col}, {diver.diver_id});
}
std::optional<int> GetDiverDiveBalance(const Diver& diver) {
  const auto kDiveCount{GetDiverDiveCount(diver)};
  if (kDiveCount < 0) {
    return {};
  }
  return diver.paid_dives - GetDiverDiveCount(diver);
}

bool IsDiverMedicalCertificateValid(const Diver& diver) {
  return diver.certif_date.addYears(1) > QDate::currentDate();
}

bool IsDiverCurrentlyRegistered(const Diver& diver) {
  return diver.registration_date.addYears(1) > QDate::currentDate();
}
bool IsDiverCurrentlyAMember(QDate diver_member_date) {
  return diver_member_date.year() == QDate::currentDate().year();
}
bool IsDiverCurrentlyAMember(const Diver& diver) {
  return IsDiverCurrentlyAMember(diver.member_date);
}

int32_t GetDiverAge(const QDate& diver_birth_date) {
  return diver_birth_date.daysTo(QDate::currentDate()) / 365;
}
int32_t GetDiverAge(const Diver& diver) {
  return GetDiverAge(diver.birth_date);
}

StoreDiverAndAddressResult StoreDiverAndItsAddress(Diver diver, const DiverAddress& address) {
  StoreDiverAndAddressResult out{};

  auto database{::db::Def()};
  if (!database.transaction()) {
    SPDLOG_ERROR("Failed to start db transaction");
    out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToStartTransaction;
    return out;
  }

  const auto kStoredAddressOpt{UpdateDiverAddress(::db::Def(), address)};
  if (!kStoredAddressOpt.has_value()) {
    SPDLOG_WARN("Failed to store address... from <StoreDiverAndItsAddress>");
    if (!database.rollback()) {
      out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToStoreElement;
    out.err_details = StoreDiverAndAddressResult::DetailErrCode::kFailedToStoreAddress;
    return out;
  }

  diver.address_id = kStoredAddressOpt->address_id;

  const auto kStoredDiverOpt{UpdateDiver(::db::Def(), diver)};
  if (!kStoredDiverOpt) {
    SPDLOG_WARN("Failed to store diver... from <StoreDiverAndItsAddress>");
    if (!database.rollback()) {
      out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToStoreElement;
    out.err_details = StoreDiverAndAddressResult::DetailErrCode::kFailedToStoreDiver;
    return out;
  }

  if (!database.commit()) {
    SPDLOG_WARN("Failed to commit transaction... from <StoreDiverAndItsAddress>");
    if (!database.rollback()) {
      out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiverAndAddressResult::ErrorCode::kFailedToCommit;
    return out;
  }

  out.err_code = StoreDiverAndAddressResult::ErrorCode::kOk;
  out.stored_diver = *kStoredDiverOpt;
  out.stored_address = *kStoredAddressOpt;

  return out;
}

bool operator==(const DiveAndDivers& lhs, const DiveAndDivers& rhs) {
  return lhs.dive == rhs.dive && lhs.members == rhs.members;
}
bool operator!=(const DiveAndDivers& lhs, const DiveAndDivers& rhs) {
  return lhs.dive != rhs.dive && lhs.members != rhs.members;
}

std::string to_string(const DiveAndDivers& dive) {
  const auto kDiveString{to_string(dive.dive)};
  std::string member_str{"["};
  for (const auto& e : dive.members) {
    member_str += to_string(e) + ",";
  }
  if (!dive.members.empty()) {
    member_str.pop_back();
  }
  member_str += "]";
  std::string out{"DiveAndDivers{.dive=" + kDiveString + ", .members=" + member_str + "}"};
  return out;
}
std::ostream& operator<<(std::ostream& os, const DiveAndDivers& dive) {
  return os << to_string(dive);
}

StoreDiveAndDiversResult StoreDiveAndItsMembers(const DiveAndDivers& dive) {
  StoreDiveAndDiversResult out{};

  auto database{::db::Def()};
  if (!database.transaction()) {
    SPDLOG_ERROR("Failed to start db transaction");
    out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToStartTransaction;
    return out;
  }

  const auto kStoredDiveOpt{UpdateDive(::db::Def(), dive.dive)};
  ROLLBACK_ON_FAIL_STORE_DIVE_AND_DIVERS(!kStoredDiveOpt,
                                         StoreDiveAndDiversResult::ErrorCode::kFailedToStoreElement,
                                         StoreDiveAndDiversResult::DetailErrCode::kFailedToStoreDive);

  bool ok_on_read_members{false};
  const auto kExistingDiveMembers{::db::readLFromDB<DiveMember>(database,
                                                                ExtractDiveMember,
                                                                "SELECT * FROM %0 WHERE %1=?",
                                                                {DiveMember::db_table, DiveMember::dive_id_col},
                                                                {kStoredDiveOpt->dive_id},
                                                                &ok_on_read_members)};
  ROLLBACK_ON_FAIL_STORE_DIVE_AND_DIVERS(!ok_on_read_members,
                                         StoreDiveAndDiversResult::ErrorCode::kFailedToReadElement,
                                         StoreDiveAndDiversResult::DetailErrCode::kFailedToReadDiveMembers);

  for (const auto& e : kExistingDiveMembers) {
    if (std::find_if(dive.members.cbegin(), dive.members.cend(), [&e](const DiveMember& lhs) {
          return lhs.diver_id == e.diver_id;
        }) != dive.members.end()) {
      /*If the diver is actually a member of the dive*/
      continue;
    }
    /* Diver should not be in the dive. Remove it */
    const auto kDeletedRemovedDiveMember{DeleteDiveMember(database, e)};
    ROLLBACK_ON_FAIL_STORE_DIVE_AND_DIVERS(!kDeletedRemovedDiveMember,
                                           StoreDiveAndDiversResult::ErrorCode::kFailedToDeleteElement,
                                           StoreDiveAndDiversResult::DetailErrCode::kFailedToDeleteDiveMember);
  }

  auto dive_members{dive.members};
  for (auto& e : dive_members) {
    e.dive_id = kStoredDiveOpt->dive_id;
  }

  for (const auto& e : dive_members) {
    const auto kStoredMemberOpt{UpdateDiveMember(::db::Def(), e)};
    ROLLBACK_ON_FAIL_STORE_DIVE_AND_DIVERS(!kStoredMemberOpt,
                                           StoreDiveAndDiversResult::ErrorCode::kFailedToStoreElement,
                                           StoreDiveAndDiversResult::DetailErrCode::kFailedToStoreDiveMember);
  }

  if (!database.commit()) {
    SPDLOG_WARN("Failed to commit transaction... from <StoreDiveAndItsMembers>");
    if (!database.rollback()) {
      out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToCommit;
    return out;
  }

  out.err_code = StoreDiveAndDiversResult::ErrorCode::kOk;
  out.stored_dive.dive = *kStoredDiveOpt;
  out.stored_dive.members = dive.members;

  return out;
}

}  // namespace cpsm::db
