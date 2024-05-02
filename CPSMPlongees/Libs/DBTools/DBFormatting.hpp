#pragma once

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>

// #include <Logger/logger.hpp>

#include "Logger/logger.hpp"

inline QString GetLastExecutedQuery(const QSqlQuery& query) {
  QString sql = query.executedQuery();
  int nbBindValues = query.boundValues().size();

  for (int i = 0, j = 0; j < nbBindValues;) {
    int s = sql.indexOf(QLatin1Char('\''), i);
    i = sql.indexOf(QLatin1Char('?'), i);
    if (i < 1) {
      break;
    }

    if (s < i && s > 0) {
      i = sql.indexOf(QLatin1Char('\''), s + 1) + 1;
      if (i < 2) {
        break;
      }
    } else {
      const QVariant& var = query.boundValue(j);
      QSqlField field(QLatin1String(""), var.metaType());
      if (var.isNull()) {
        field.clear();
      } else {
        field.setValue(var);
      }
      QString formatV = query.driver()->formatValue(field);
      sql.replace(i, 1, formatV);
      i += formatV.length();
      ++j;
    }
  }

  return sql;
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
