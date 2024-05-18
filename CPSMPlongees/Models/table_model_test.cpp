#include "table_model_test.hpp"

#include <QBrush>
#include <QFont>

#include <Logger/logger.hpp>

#include "Constants.hpp"

MyModel::MyModel(QObject *parent) : QAbstractTableModel(parent) {}

int MyModel::rowCount(const QModelIndex & /*parent*/) const {
  return 2;
}

int MyModel::columnCount(const QModelIndex & /*parent*/) const {
  return 3;
}

QVariant MyModel::headerData(int section, Qt::Orientation orientation, int role) const {
  SPDLOG_DEBUG("===== INFO return: role=<{}> orientation=<{}> section=<{}>", role, orientation, section);
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("first");
      case 1:
        return QString("second");
      case 2:
        return QString("third");
    }
  }
  SPDLOG_ERROR("===== EMPTY return: role=<{}> orientation=<{}> section=<{}>", role, orientation, section);
  return QVariant();
}

QVariant MyModel::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called
  SPDLOG_DEBUG("--------------------- row {}, {}, role {}", row, col, role);

  switch (role) {
    case Qt::DisplayRole:
      if (row == 0 && col == 1) return QString("<--left");
      if (row == 1 && col == 1) return QString("right-->");

      return QString("Row%1, Column%2").arg(row + 1).arg(col + 1);
    case Qt::FontRole:
      if (row == 0 && col == 0) {  // change font only for cell(0,0)
        QFont boldFont;
        boldFont.setBold(true);
        return boldFont;
      }
      break;
    case Qt::BackgroundRole:
      if (row == 1 && col == 2)  // change background only for cell(1,2)
        return QBrush(Qt::red);
      break;
    case Qt::TextAlignmentRole:
      if (row == 1 && col == 1)  // change text alignment only for cell(1,1)
        return int(Qt::AlignRight | Qt::AlignVCenter);
      break;
    case Qt::CheckStateRole:
      if (row == 1 && col == 0)  // add a checkbox to cell(1,0)
        return Qt::Checked;
      break;
  }
  return QVariant();
}
