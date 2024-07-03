#include "RawStructs.hpp"

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
    return {};
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
  if (!kStoredDiveOpt) {
    SPDLOG_WARN("Failed to store dive... from <StoreDiveAndItsMembers>");
    if (!database.rollback()) {
      out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToStoreElement;
    out.err_details = StoreDiveAndDiversResult::DetailErrCode::kFailedToStoreDive;
    return {};
  }

  for (const auto& e : dive.members) {
    const auto kStoredMemberOpt{UpdateDiveMember(::db::Def(), e)};
    if (!kStoredMemberOpt) {
      SPDLOG_WARN("Failed to store dive member... from <StoreDiveAndItsMembers>");
      if (!database.rollback()) {
        out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToRollback;
        return out;
      } /* else */
      out.err_code = StoreDiveAndDiversResult::ErrorCode::kFailedToStoreElement;
      out.err_details = StoreDiveAndDiversResult::DetailErrCode::kFailedToStoreDiveMember;
      return {};
    }
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
