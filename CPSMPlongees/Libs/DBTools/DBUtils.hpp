#pragma once

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QString>

#include <Utils/chrono.hpp>

#include "DBFormatting.hpp"

namespace db {

namespace internal_tools {
inline int CountMatches(const QString &text, const QRegularExpression &re) {
  int count{};
  QRegularExpressionMatchIterator i = re.globalMatch(text);
  while (i.hasNext()) {
    i.next();
    count++;
  }
  return count;
}
}  // namespace internal_tools

inline auto Def() {
  return QSqlDatabase::database();
}

inline bool OpenLocal(const QString &fileName, const QString &connectionName = "") {
  QSqlDatabase db;
  if (connectionName.isEmpty())
    db = QSqlDatabase::addDatabase("QSQLITE");
  else
    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
  // db.setHostName("localhost");
  db.setDatabaseName(fileName);
  if (!db.open()) {
    SPDLOG_ERROR("Cannot open db: {}", db.lastError());

    return false;
  }

  SPDLOG_INFO("Opened local db<{}> with driver<{}>", fileName, db.driverName());

  return true;
}

inline QString ResultToString(QSqlQuery *query) {
  QString out{};
  const auto kColCount{query->record().count()};
  QString header{};
  for (int32_t i{}; i < kColCount; ++i) {
    header += query->record().fieldName(i) + " ";
  }
  out += header + "\n";
  while (query->next()) {
    QString line{};
    for (int32_t i{}; i < kColCount; ++i) {
      line += "\"" + query->value(i).toString() + "\" ";
    }
    out += line + "\n";
  }
  return out;
}

/*!
 * \brief Execute a SQL script
 * @warning It requires each statement which should be executed independantly to start on a new line
 * @note Each line ending with a ";" will be considered as an ending individual SQL query
 * @note If a query contains "--", anything following it will be considered as a comment
 *
 * Ex: SELECT * FROM Table; SELECT * FROM Table1;
 * Will be considered as one unique request. Whereas
 * Ex:
 *      SELECT * FROM Table;
 *      SELECT * FROM Table1;
 * Will be considered as two different queries
 *
 * \param file_name
 * \param db
 * \return
 */
template <bool kPrintResults = false, bool kPrintSuccessfulQuery = false>
[[nodiscard]] bool ExecScript(const QString &file_name, QSqlDatabase *db) {
  SPDLOG_INFO("Executing SQL script: {}", file_name);

  utils::Chrono<std::chrono::milliseconds> chrono{};

  QFile file(file_name);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    SPDLOG_INFO("Could not open SQL script");
    return false;
  }

  QTextStream in(&file);
  QString sql = in.readAll();
  file.close();

  static const QRegularExpression regex_comment{
      R"(\s*--.*$)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption};
  static const QRegularExpression regex_query_end{
      R"(\s*;\s*$)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption};
  static const QRegularExpression regex_begin_in_query{
      R"(\bbegin\b)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption};
  static const QRegularExpression regex_end_in_query{
      R"(\bend\b)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption};
  static const QRegularExpression regex_check_file_end{
      R"(\w{2,})", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption};

  sql.replace(regex_comment, "");

  std::vector<std::tuple<QString, int>> queries{};

  QString current_query{};
  int32_t line_index{};
  int32_t starting_line_index{-1};
  int32_t begin_count{};
  int32_t end_count{};
  for (const auto &line : sql.split('\n', Qt::KeepEmptyParts)) {
    ++line_index;
    if (line.isEmpty()) {
      continue;
    }
    if (regex_comment.match(line).hasMatch()) {
      continue;
    }

    begin_count += internal_tools::CountMatches(line, regex_begin_in_query);
    end_count += internal_tools::CountMatches(line, regex_end_in_query);

    if (starting_line_index < 0) {
      starting_line_index = line_index;
    }
    current_query += "\n" + line;

    if (regex_query_end.match(line).hasMatch() && begin_count == end_count) {
      queries.emplace_back(current_query, starting_line_index);
      current_query.clear();
      starting_line_index = -1;
      begin_count = 0;
      end_count = 0;
    }
  }
  if (regex_check_file_end.match(current_query).hasMatch()) {
    SPDLOG_WARN("Script may not close the last query properly: {}", current_query);
  }
  if (begin_count != end_count) {
    SPDLOG_WARN("Script may not close the last query properly - BEGIN count({}) != END count({}) : {}",
                begin_count,
                end_count,
                current_query);
  }

  int success_count{};
  bool success{true};
  QSqlQuery query(*db);
  for (const auto &[query_str, line] : queries) {
    utils::Chrono<std::chrono::milliseconds> query_chrono{};
    const auto query_success{query.exec(query_str)};
    double query_duration_s{query_chrono.Time() / 1000.};
    if (query_success) {
      if constexpr (kPrintSuccessfulQuery) {
        SPDLOG_DEBUG("\tQuery (Succeded in {}s) l.{}: {}", query_duration_s, line, query_str);
      }
      if constexpr (kPrintResults) {
        SPDLOG_INFO("{}", ResultToString(&query));
      }
    } else {
      SPDLOG_DEBUG(
          "\tQuery (Failed in {}s) "
          "l.{}:\n---------------------------------------{}\n---------------------------------------\n=> Error: {}",
          query_duration_s,
          line,
          query_str,
          query.lastError());
    }

    success = success && query_success;
    if (query_success) success_count += 1;
  }

  SPDLOG_INFO("Successful query count: {}/{} in {:.2f}s", success_count, queries.size(), chrono.Time() / 1000.);

  return success;
}

/*!
 * \brief formatListForSQL : format a list like `{1,2,3}` in `"(1,2,3)"`
 * \param in : input list
 * \return formatted string
 */
template <typename T>
inline QString formatListForSQL(const QVector<QVariant> &in) {
  QString out{'('};
  for (const auto &e : in) {
    out = out + QString("%0,").arg(e.value<T>());
  }
  if (out.endsWith(',')) out.resize(out.size() - 1);
  out += ")";
  return out;
}

template <typename T>
inline QString formatListForSQL(const QVector<T> &in) {
  QString out{'('};
  for (const auto &e : in) {
    out = out + QString("%0,").arg(e);
  }
  if (out.endsWith(',')) out.resize(out.size() - 1);
  out += ")";
  return out;
}

/*!
 * \brief questionMarkList : create a list of questino mark for SQL requests.
 *              For example : `{1,3,6,9}` will produce `"(?,?,?,?)"`
 * \param in
 * \return
 */
template <typename T>
inline QString questionMarkList(const QVector<T> &in) {
  QString out{'('};
  for (const auto &e : in) {
    std::ignore = e;
    out = out + "?,";
  }
  if (out.endsWith(',')) out.resize(out.size() - 1);
  out += ")";
  return out;
}

/*!
 * \brief prepRequestListFilter : prepare a list of elements for a SQL request
 * \param in : input list
 * \return a formatted string like (?,?,?,?) and a vector<QVariant> containing values to use in the sql req
 */
template <typename T>
inline std::tuple<QString, QVector<QVariant>> prepRequestListFilter(const QVector<T> &in) {
  auto ouStr{questionMarkList(in)};
  QVector<QVariant> outL(in.size());
  for (int i{}; i < outL.size(); ++i) {
    outL[i] = QVariant{in[i]};
  }
  return {ouStr, outL};
}

}  // namespace db
