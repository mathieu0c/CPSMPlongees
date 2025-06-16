#include "DivesViewModelForDiverEdit.hpp"

#include <QFont>

#include "CPSMGlobals.hpp"
#include "Utils.hpp"

namespace cpsm {

DivesViewModelForDiverEdit::DivesViewModelForDiverEdit(QObject *parent) : DivesViewModel{parent} {
  //
}

int DivesViewModelForDiverEdit::columnCount(const QModelIndex & /*parent*/) const {
  return kColumnsHeaders.size();
}

QVariant DivesViewModelForDiverEdit::headerData(int section, Qt::Orientation orientation, int role) const {
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
      return int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::FontRole: {
      static QFont default_font{};
      return default_font;
      break;
    }
  }
  return QVariant();
}

QVariant DivesViewModelForDiverEdit::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called

  static const DisplayDive kEmptyDive{};
  const auto &dive{row < GetDisplayDives().size() ? GetDisplayDives().at(row) : kEmptyDive};

  switch (role) {
    case Qt::DisplayRole:
      return GetDisplayTextForIndex(dive, col);
    case Qt::FontRole:
      break;
    case Qt::BackgroundRole:
      return GetBackgroundForIndex(dive, col);
    case Qt::TextAlignmentRole:
      return int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::CheckStateRole:
      break;
  }
  return QVariant();
}

QString DivesViewModelForDiverEdit::GetDisplayTextForIndex(const DisplayDive &dive, int col) const {
  switch (col) {
    case ColumnId::kDate: {
      return dive.dive.datetime.date().toString(cpsm::consts::kDateUserFormat);
    }
    case ColumnId::kTime: {
      return dive.dive.datetime.time().toString(cpsm::consts::kTimeFormat);
    }
    case ColumnId::kSite: {
      return GetDivingSiteText(dive.dive.diving_site_id);
    }
    case ColumnId::kType: {
      return dive.dive_types;
    }
    default:
      SPDLOG_WARN("Unknown column title index: <{}>", col);
      break;
  }
  return tr("ERREUR - inconnu");
}

QVariant DivesViewModelForDiverEdit::GetBackgroundForIndex(const DisplayDive &complete_dive, int col) const {
  const auto &dive{complete_dive.dive};
  switch (col) {
    case ColumnId::kDate: {
      return dive.datetime.date() == QDate::currentDate() ? ::consts::colors::kBackgroundGreen
                                                          : ::consts::colors::kBackgroundInvisible;
    }
    case ColumnId::kTime: {
      return BackgroundColorFromTime(dive.datetime.time());
    }
    default:
      break;
  }
  return {};
}

}  // namespace cpsm

#include "moc_DivesViewModelForDiverEdit.cpp"
