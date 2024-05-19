#pragma once

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>

#include "Logger/logger.hpp"

inline QString GetLastExecutedQuery(const QSqlQuery& query) {
  QString str = query.executedQuery();

  const auto kBoundValues{query.boundValues()};
  if (kBoundValues.size() == 0) {
    return str;
  }

  for (int i{static_cast<int>(query.boundValues().size() - 1)}; i >= 0; --i) {
    const auto kBoundValue{query.boundValue(i)};
    const auto kBoundValueName{query.boundValueName(i)};

    if (kBoundValue.isNull()) {
      str.replace(kBoundValueName, "NULL");
    } else {
      str.replace(kBoundValueName, kBoundValue.toString());
    }
  }
  return str;
}

DECLARE_CUSTOM_SPD_FORMAT(QSqlError) {
  return stream << val.text();
}

DECLARE_CUSTOM_SPD_FORMAT(QSqlQuery) {
  auto err{val.lastError()};

  stream << " --------- SQL Query ---------\n";
  stream << GetLastExecutedQuery(val).toStdString() << "\n";
  stream << "Error: <" << err.text().toStdString() << ">\n";
  stream << " --------- END SQL Query ---------\n";

  return stream;
}
