#include "table_model_test.hpp"

#include <QBrush>
#include <QFont>

#include <Logger/logger.hpp>

MyModel::MyModel(QObject *parent) : QAbstractTableModel(parent) {}

int MyModel::rowCount(const QModelIndex & /*parent*/) const {
  return 2;
}

int MyModel::columnCount(const QModelIndex & /*parent*/) const {
  return 3;
}

QVariant MyModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

// ----------------------------
// ----------------------------
// ----------------------------
// ----------------------------
// ----------------------------
namespace cpsm {
DiverTableModel::DiverTableModel(QObject *parent) : QAbstractTableModel(parent) {}

void DiverTableModel::LoadFromDB() {
  SPDLOG_DEBUG("Loading from db");
  auto list{db::readLFromDB<db::Diver>(db::Def(), db::ExtractDiver, "SELECT * FROM %0", {db::Diver::db_table}, {})};
  SPDLOG_DEBUG("Read from db: ");

  SetDivers(std::move(list));
}

void DiverTableModel::SetDivers(QVector<db::Diver> divers) {
  m_divers = std::move(divers);
  SetDiversToDisplay(m_divers);
}

void DiverTableModel::SetDiversToDisplay(QVector<db::Diver> divers) {
  beginRemoveRows({}, 0, rowCount() - 1);
  endRemoveRows();
  m_divers_to_display = divers;
  beginInsertRows({}, 0, divers.size());
  endInsertRows();
}

int DiverTableModel::rowCount(const QModelIndex & /*parent*/) const {
  return m_divers_to_display.size();
}

int DiverTableModel::columnCount(const QModelIndex & /*parent*/) const {
  return 3;
}

QVariant DiverTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
  return QVariant();
}

QVariant DiverTableModel::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called

  if (col == 0) {
    return QString{m_divers_to_display[row].last_name.toUpper()} + " " + QString{m_divers_to_display[row].first_name};
  }

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
}  // namespace cpsm
