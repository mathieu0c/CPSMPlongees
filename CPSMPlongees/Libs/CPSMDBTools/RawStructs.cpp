#include "RawStructs.hpp"

namespace db {

int GetDiverDiveCount(const Diver& diver) {
  auto db{db::Def()};
  return db::queryCount(
      db, "SELECT diver_id FROM %0 WHERE %1 = ?", {DiveMember::db_table, DiveMember::diver_id_col}, {diver.diver_id});
}

StoreDiverAndAddressResult StoreDiverAndItsAddress(Diver diver, const DiverAddress& address) {
  StoreDiverAndAddressResult out{};

  auto database{db::Def()};
  if (!database.transaction()) {
    SPDLOG_ERROR("Failed to start db transaction");
    out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToStartTransaction;
    return out;
  }

  const auto kStoredAddressOpt{UpdateDiverAddress(db::Def(), address)};
  if (!kStoredAddressOpt.has_value()) {
    SPDLOG_WARN("Failed to store address... from <StoreDiverAndItsAddress>");
    if (!database.rollback()) {
      out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToStoreAddress;
    return out;
  }

  diver.address_id = kStoredAddressOpt->address_id;

  const auto kStoredDiverOpt{UpdateDiver(db::Def(), diver)};
  if (!kStoredDiverOpt) {
    SPDLOG_WARN("Failed to store diver... from <StoreDiverAndItsAddress>");
    if (!database.rollback()) {
      out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToStoreDiver;
    return {};
  }

  if (!database.commit()) {
    SPDLOG_WARN("Failed to commit transaction... from <StoreDiverAndItsAddress>");
    if (!database.rollback()) {
      out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToRollback;
      return out;
    } /* else */
    out.err_code = StoreDiverAndAddressResult::ErrCode::kFailedToCommit;
    return out;
  }

  out.err_code = StoreDiverAndAddressResult::ErrCode::kOk;
  out.stored_diver = *kStoredDiverOpt;
  out.stored_address = *kStoredAddressOpt;

  return out;
}

}  // namespace db
