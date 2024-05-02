#pragma once

#include <QString>
#include <sstream>

#include <Logger/logger.hpp>
#include <spdlog/fmt/ostr.h>

namespace db {

namespace internals {
inline QString Join(const QStringList &list, const QString &separator) {
  if (list.empty()) {
    return QString{};
  }

  QString out{};
  for (const auto &str : list) {
    out += str + separator;
  }
  out.chop(separator.size());
  return out;
}
}  // namespace internals

/* To declare a new db struct, here is an example:
 * Function is COLUMN(name, DB_TYPE, IS_PRIMARY)

#define ClassName_VAR_LIST(COLUMN)        \
    COLUMN(diver_id, INTEGER, true;       \
    COLUMN(first_name, TEXT);        \
    COLUMN(gear_regulator, INTEGER); \
    COLUMN(gear_suit, INTEGER);      \
    COLUMN(gear_computer, INTEGER);
DB_DECLARE_STRUCT(TestStruct, ClassName_VAR_LIST)

This declares a struct with:
- A member variable of the given name and corresponding
- For each member variable, an associated "name_col" raw string corresponding to the column name
- A stream operator to display the value
- An ExtractClassName to feed into a db::readLFromDB as an extractor function
 */

template <typename T>
constexpr bool IsString() {
  return std::is_same_v<T, std::string> || std::is_same_v<T, QString>;
}

enum DBType { INTEGER, TEXT, DATE, DATETIME };

template <typename T>
struct DBTypeWrapper {
 public:
  using Type = T;
};

template <DBType kDBType>
constexpr auto GetTypeForDBType()

{
  if constexpr (kDBType == DBType::INTEGER) {
    return DBTypeWrapper<int>{};
  } else if constexpr (kDBType == DBType::TEXT) {
    return DBTypeWrapper<QString>{};
  } else if constexpr (kDBType == DBType::DATE) {
    return DBTypeWrapper<QDate>{};
  } else if constexpr (kDBType == DBType::DATETIME) {
    return DBTypeWrapper<QDateTime>{};
  } else {
    return DBTypeWrapper<int>{};
  }
}

template <DBType kDBType>
using GetTypeFromDB = typename decltype(GetTypeForDBType<kDBType>())::Type;

#define DB_ELEM(name, db_type, IS_PRIMARY) \
  GetTypeFromDB<db_type> name{};           \
  static constexpr auto name##_col {       \
    #name                                  \
  }

#define DB_PRINT_NAME_AND_VALUE(name, DB_TYPE, IS_PRIMARY) \
  os << "." #name "=";                                     \
  if constexpr (IsString<decltype(val.name)>()) {          \
    os << "\"" << val.name << "\",";                       \
  } else {                                                 \
    os << val.name << ",";                                 \
  }

#define DB_STREAM_OPS(ClassName, LIST_OF_VARIABLES_MACRO)                   \
  inline std::ostream &operator<<(std::ostream &os, const ClassName &val) { \
    os << to_string(val);                                                   \
    return os;                                                              \
  }

#define DB_TO_STRING_OPS(ClassName, LIST_OF_VARIABLES_MACRO) \
  inline std::string to_string(const ClassName &val) {       \
    std::stringstream os{};                                  \
    os << #ClassName "{";                                    \
    LIST_OF_VARIABLES_MACRO(DB_PRINT_NAME_AND_VALUE)         \
    os.seekp(-1, std::ios_base::cur);                        \
    os << "}";                                               \
    return os.str();                                         \
  }

// #define DB_EXTRACT_FUNCTION_LINE(name, DB_TYPE, IS_PRIMARY) out.name =
// q.value(out.name##_col).value<GetTypeFromDB<DB_TYPE>>()
#define DB_EXTRACT_FUNCTION_LINE(name, DB_TYPE, IS_PRIMARY)  \
  if (auto i = q.record().indexOf(out.name##_col); i > -1) { \
    out.name = q.value(i).value<GetTypeFromDB<DB_TYPE>>();   \
  }
#define DB_EXTRACT_FUNCTION(ClassName, LIST_OF_VARIABLES_MACRO) \
  inline ClassName Extract##ClassName(const QSqlQuery &q) {     \
    ClassName out{};                                            \
    LIST_OF_VARIABLES_MACRO(DB_EXTRACT_FUNCTION_LINE)           \
    return out;                                                 \
  }

#define DB_INSERT_OR_UPDATE_STEP_COLUMNS_LIST(name, DB_TYPE, IS_PRIMARY) columns_list.append(val.name##_col);
#define DB_INSERT_OR_UPDATE_STEP_VAL_LIST(name, DB_TYPE, IS_PRIMARY) val_list.append(val.name);
#define DB_INSERT_OR_UPDATE_STEP_VAL_LIST_PK(name, DB_TYPE, IS_PRIMARY) \
  if constexpr (IS_PRIMARY) val_list.append(val.name);
#define DB_INSERT_OR_UPDATE_STEP_PK_LIST(name, DB_TYPE, IS_PRIMARY) \
  if constexpr (IS_PRIMARY) primary_key_list.append(val.name##_col);

#define DB_INSERT_OR_UPDATE_FUNCTION(ClassName, invalid_val_condition, LIST_OF_VARIABLES_MACRO)        \
  [[nodiscard]] inline bool Update##ClassName(QSqlDatabase db, const ClassName &val) {                 \
    if (invalid_val_condition) {                                                                       \
      /* Cannot insert or update an invalid value*/                                                    \
      SPDLOG_DEBUG("Failed to validate condition !({}) with object: {}", #invalid_val_condition, val); \
      return false;                                                                                    \
    }                                                                                                  \
                                                                                                       \
    QVector<QVariant> val_list{};                                                                      \
    LIST_OF_VARIABLES_MACRO(DB_INSERT_OR_UPDATE_STEP_VAL_LIST)                                         \
    LIST_OF_VARIABLES_MACRO(DB_INSERT_OR_UPDATE_STEP_VAL_LIST_PK)                                      \
                                                                                                       \
    static QString base_request{};                                                                     \
    if (base_request.isEmpty()) {                                                                      \
      QStringList columns_list{};                                                                      \
      LIST_OF_VARIABLES_MACRO(DB_INSERT_OR_UPDATE_STEP_COLUMNS_LIST)                                   \
                                                                                                       \
      static QStringList primary_key_list{};                                                           \
      LIST_OF_VARIABLES_MACRO(DB_INSERT_OR_UPDATE_STEP_PK_LIST)                                        \
                                                                                                       \
      QString where_clause{};                                                                          \
      if (!primary_key_list.empty()) {                                                                 \
        where_clause += "WHERE ";                                                                      \
        const auto kWhereData{internals::Join(primary_key_list, "=? AND ")};                           \
        where_clause += kWhereData;                                                                    \
        where_clause += "=?";                                                                          \
      }                                                                                                \
                                                                                                       \
      QString affect_excluded{};                                                                       \
      for (const auto &e : columns_list) {                                                             \
        affect_excluded += e + "=excluded." + e + ",";                                                 \
      }                                                                                                \
      affect_excluded.removeLast();                                                                    \
                                                                                                       \
      auto actual_value_list_for_question_marks{val_list};                                             \
      actual_value_list_for_question_marks.resize(val_list.size() - primary_key_list.size());          \
                                                                                                       \
      base_request = "INSERT INTO %0%1 VALUES%2 ON CONFLICT(%3) DO UPDATE SET %4 %5";                  \
      QStringList str_list{};                                                                          \
      str_list.emplaceBack(val.db_table);                                               /* #0*/        \
      str_list.emplaceBack("(" + internals::Join(columns_list, ",") + ")");             /* #1*/        \
      str_list.emplaceBack(db::questionMarkList(actual_value_list_for_question_marks)); /* #2*/        \
      str_list.emplaceBack(internals::Join(primary_key_list, ","));                     /* #3*/        \
      str_list.emplaceBack(affect_excluded);                                            /* #4*/        \
      str_list.emplaceBack(where_clause);                                               /* #5*/        \
                                                                                                       \
      for (const auto &e : str_list) {                                                                 \
        base_request = base_request.arg(e);                                                            \
      }                                                                                                \
      SPDLOG_DEBUG("Base request: <{}>", base_request);                                                \
    }                                                                                                  \
                                                                                                       \
    return db::ExecQuery(db, base_request, {}, val_list);                                              \
  }

#define DB_FUNCTION_STEP_PRIMARY_KEY_LIST(name, DB_TYPE, IS_PRIMARY) \
  if constexpr (IS_PRIMARY) {                                        \
    primary_key_list.append(val.name##_col);                         \
  }
#define DB_FUNCTION_STEP_VAL_LIST(name, DB_TYPE, IS_PRIMARY) \
  if constexpr (IS_PRIMARY) {                                \
    val_list.append(val.name);                               \
  }

#define DB_DELETE_FUNCTION(ClassName, invalid_val_condition, LIST_OF_VARIABLES_MACRO)                  \
  [[nodiscard]] inline bool Delete##ClassName(QSqlDatabase db, const ClassName &val) {                 \
    if (invalid_val_condition) {                                                                       \
      /* Cannot insert or update an invalid value*/                                                    \
      SPDLOG_DEBUG("Failed to validate condition !({}) with object: {}", #invalid_val_condition, val); \
      return false;                                                                                    \
    }                                                                                                  \
                                                                                                       \
    QVector<QVariant> val_list{};                                                                      \
    LIST_OF_VARIABLES_MACRO(DB_FUNCTION_STEP_VAL_LIST)                                                 \
                                                                                                       \
    static QStringList primary_key_list{};                                                             \
    static QString request{"DELETE FROM %0 WHERE "};                                                   \
    if (primary_key_list.empty()) {                                                                    \
      LIST_OF_VARIABLES_MACRO(DB_FUNCTION_STEP_PRIMARY_KEY_LIST)                                       \
                                                                                                       \
      if (primary_key_list.empty()) {                                                                  \
        SPDLOG_CRITICAL("Need at least one primary key to delete for " #ClassName);                    \
        throw std::runtime_error("No primary key found for " #ClassName);                              \
      }                                                                                                \
                                                                                                       \
      request = request.arg(ClassName::db_table);                                                      \
                                                                                                       \
      for (int i{}; i < primary_key_list.size(); i++) {                                                \
        request += primary_key_list[i] + " = ?";                                                       \
        if (i < primary_key_list.size() - 1) {                                                         \
          request += " AND ";                                                                          \
        }                                                                                              \
      }                                                                                                \
      request += ";";                                                                                  \
    }                                                                                                  \
    SPDLOG_INFO("Delete request: {}", request);                                                        \
                                                                                                       \
    return db::queryDelete(db, request, {}, val_list);                                                 \
  }

#define DB_GET_FROM_ID_FUNCTION(ClassName, LIST_OF_VARIABLES_MACRO)                                                    \
  [[nodiscard]] inline std::optional<ClassName> Get##ClassName##FromId(QSqlDatabase db,                                \
                                                                       const QVector<QVariant> &primary_keys_values) { \
    static const ClassName val{}; /*NOLINT*/                                                                           \
    static QStringList primary_key_list{};                                                                             \
    static QString request{"SELECT * FROM %0 WHERE "};                                                                 \
    if (primary_key_list.empty()) {                                                                                    \
      LIST_OF_VARIABLES_MACRO(DB_FUNCTION_STEP_PRIMARY_KEY_LIST)                                                       \
                                                                                                                       \
      if (primary_key_list.empty()) {                                                                                  \
        SPDLOG_CRITICAL("Need at least one primary key to delete for " #ClassName);                                    \
        throw std::runtime_error("No primary key found for " #ClassName);                                              \
      }                                                                                                                \
                                                                                                                       \
      request = request.arg(ClassName::db_table);                                                                      \
                                                                                                                       \
      for (int i{}; i < primary_key_list.size(); i++) {                                                                \
        request += primary_key_list[i] + " = ?";                                                                       \
        if (i < primary_key_list.size() - 1) {                                                                         \
          request += " AND ";                                                                                          \
        }                                                                                                              \
      }                                                                                                                \
      request += ";";                                                                                                  \
    }                                                                                                                  \
    SPDLOG_INFO("Read from request: {}", request);                                                                     \
                                                                                                                       \
    return db::readFromDB<ClassName>(db, db::Extract##ClassName, request, {}, primary_keys_values);                    \
  }

#define DB_EQUAL_OPERATOR_FUNCTION_STEP(name, DB_TYPE, IS_PRIMARY) \
  if (lhs.name != rhs.name) return false

#define DB_EQUAL_OPERATOR_FUNCTION(ClassName, LIST_OF_VARIABLES_MACRO) \
  inline bool operator==(const ClassName &lhs, const ClassName &rhs) { \
    LIST_OF_VARIABLES_MACRO(DB_EQUAL_OPERATOR_FUNCTION_STEP)           \
    return true;                                                       \
  }

/*!
 *  @param ClassName: is the struct to be declared
 *  @param AssociatedTableName: the table in which this datastruct is stored
 *  @param invalid_val_condition: the condition which will invalidate an insert or update command.
 *  Usually is something like `val.primary_key <= 0` Note that "val" is of type const ClassName&
 *  and the name of this variable is imposed
 *  @param FOR_DB_VAR_LIST: list of declared variables such as:
 *  #define DiveMember_VAR_LIST(COLUMN) \
        COLUMN(dive_id, INTEGER);         \
        COLUMN(diver_id, INTEGER);        \
        COLUMN(diving_type_id, INTEGER);
 *  @example
        DB_DECLARE_STRUCT(DiveMember, DivesMembers, val.dive_id <= 0 || val.diver_id <= 0 , DiveMember_VAR_LIST)
 */
#define DB_DECLARE_STRUCT(ClassName, AssociatedTableName, invalid_val_condition, FOR_DB_VAR_LIST) \
  struct ClassName {                                                                              \
    static constexpr auto kClassName{#ClassName};                                                 \
    static constexpr auto db_table{#AssociatedTableName};                                         \
    FOR_DB_VAR_LIST(DB_ELEM)                                                                      \
  };                                                                                              \
  DB_TO_STRING_OPS(ClassName, FOR_DB_VAR_LIST)                                                    \
  DB_STREAM_OPS(ClassName, FOR_DB_VAR_LIST)                                                       \
  DB_EXTRACT_FUNCTION(ClassName, FOR_DB_VAR_LIST)                                                 \
  DB_INSERT_OR_UPDATE_FUNCTION(ClassName, invalid_val_condition, FOR_DB_VAR_LIST)                 \
  DB_DELETE_FUNCTION(ClassName, invalid_val_condition, FOR_DB_VAR_LIST)                           \
  DB_GET_FROM_ID_FUNCTION(ClassName, FOR_DB_VAR_LIST)                                             \
  DB_EQUAL_OPERATOR_FUNCTION(ClassName, FOR_DB_VAR_LIST)

}  // namespace db
