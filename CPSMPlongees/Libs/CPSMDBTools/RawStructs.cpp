#include "RawStructs.hpp"

namespace db {

int GetDiverDiveCount(const Diver &diver) {
  auto db{db::Def()};
  return db::queryCount(
      db, "SELECT diver_id FROM %0 WHERE %1 = ?", {DiveMember::db_table, DiveMember::diver_id_col}, {diver.diver_id});
}

}  // namespace db
