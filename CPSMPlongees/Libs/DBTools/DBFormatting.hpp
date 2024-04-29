#pragma once

#include <QSqlError>
#include <QSqlQuery>

// #include <Logger/logger.hpp>

#include "Logger/logger.hpp"

DECLARE_CUSTOM_SPD_FORMAT(QSqlError) {
  return stream << val.text();
}

DECLARE_CUSTOM_SPD_FORMAT(QSqlQuery) {
  auto err{val.lastError()};
  auto qText{val.lastQuery()};
  stream << ": error: " << err << "  >>>";
  for (const auto& e : val.boundValues()) {
    qText = qText.replace(qText.indexOf('?'), 1, e.toString());
  }
  stream << "    " << qText;

  return stream;
}
