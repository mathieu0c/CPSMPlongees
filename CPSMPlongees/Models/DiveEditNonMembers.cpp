#include "DiveEditNonMembers.hpp"

#include "Constants.hpp"

namespace cpsm {

DiveEditNonMembers::DiveEditNonMembers(QObject *parent) : DiversViewModel{parent} {}

void DiveEditNonMembers::LoadFromDB() {
  return DiversViewModel::LoadFromDB();
}

QVariant DiveEditNonMembers::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal) {
    return {};
  }

  switch (role) {
    case Qt::DisplayRole:
      if (static_cast<size_t>(section) < kColumnsHeaders.size()) {
        return kColumnsHeaders[section];
      } else {
        break;
      }
    case Qt::TextAlignmentRole:
      return section == ColumnId::kBalance ? int(Qt::AlignLeft | Qt::AlignVCenter)
                                           : int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::FontRole: {
      static QFont default_font{};
      return default_font;
      break;
    }
  }
  return QVariant();
}

QVariant DiveEditNonMembers::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called

  static const DiverWithDiveCount kEmptyDiver{};
  const auto &diver{row < DiversToDisplay().size() ? DiversToDisplay().at(row) : kEmptyDiver};

  switch (role) {
    case Qt::DisplayRole:
      return GetDisplayTextForIndex(diver, col);
    case Qt::FontRole:
      // if (row == 0 && col == 0) {  // change font only for cell(0,0)
      //     QFont boldFont;
      //     boldFont.setBold(true);
      //     return boldFont;
      // }
      break;
    case Qt::BackgroundRole:
      return GetBackgroundForIndex(diver, col);
    case Qt::TextAlignmentRole:
      return col == ColumnId::kBalance ? int(Qt::AlignLeft | Qt::AlignVCenter)
                                       : int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::CheckStateRole:
      // if (row == 1 && col == 0)  // add a checkbox to cell(1,0)
      //   return Qt::Checked;
      break;
  }
  return QVariant();
}

int DiveEditNonMembers::columnCount(const QModelIndex & /*parent*/) const {
  return kColumnsHeaders.size();
}

QString DiveEditNonMembers::GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, int col) const {
  const auto &diver{complete_diver.diver};

  switch (col) {
    case ColumnId::kLastName: {
      return diver.last_name;
    }
    case ColumnId::kFirstName: {
      return diver.first_name;
    }
    case ColumnId::kLevel: {
      return GetLevelText(diver.diver_level_id);
    }
    case ColumnId::kBalance: {
      return QString::number(complete_diver.Balance());
    }
    default:
      SPDLOG_WARN("Unknown column title index: <{}>", col);
      break;
  }
  return tr("ERREUR - inconnu");
}

QVariant DiveEditNonMembers::GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, int col) const {
  auto lambda_bool{[](bool b) { return b ? ::consts::colors::kBackgroundGreen : ::consts::colors::kBackgroundRed; }};

  switch (col) {
    case ColumnId::kBalance: {
      return lambda_bool(complete_diver.Balance() >= 0);
    }
    default:
      break;
  }
  return {};
}

}  // namespace cpsm

#include "moc_DiveEditNonMembers.cpp"
