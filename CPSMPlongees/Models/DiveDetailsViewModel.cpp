#include "DiveDetailsViewModel.hpp"

#include <CPSMGlobals.hpp>
#include <Constants.hpp>
#include <QFont>

namespace cpsm {

DiveDetailsViewModel::DiveDetailsViewModel(QObject *parent) : QAbstractTableModel(parent) {}

void DiveDetailsViewModel::LoadFromDB(int dive_id) {
  m_db_diving_types.clear();

  const auto kDivingTypeList{::db::readLFromDB<db::DivingType>(
      ::db::Def(), db::ExtractDivingType, "SELECT * FROM %0", {db::DivingType::db_table}, {})};

  if (kDivingTypeList.size() == 0) {
    SPDLOG_ERROR("Failed to load diving types from db. Retrying... (Or first attempt)");
  }
  for (const auto &type : kDivingTypeList) {
    m_db_diving_types[type.diving_type_id] = type;
  }

  SPDLOG_TRACE("Reloading DiveDetailsViewModel from db");

  const QString kBaseQuery{R"(SELECT * FROM %0 NATURAL JOIN %1 WHERE %2=?)"};
  const QStringList kStrArgs{
      db::DiveMember::db_table,   /* 0 */
      db::Diver::db_table,        /* 1 */
      db::DiveMember::dive_id_col /* 2 */
  };
  const QVariantList kArgs{dive_id};

  auto list{::db::readLFromDB<DiveDetailsDiver>(
      ::db::Def(),
      [&](const QSqlQuery &query) {
        auto dive_member{db::ExtractDiveMember(query)};
        auto diver_first_name{query.value(db::Diver::first_name_col).toString()};
        auto diver_last_name{query.value(db::Diver::last_name_col).toString()};
        return DiveDetailsDiver{.member = std::move(dive_member),
                                .first_name = std::move(diver_first_name),
                                .last_name = std::move(diver_last_name)};
      },
      kBaseQuery,
      kStrArgs,
      kArgs)};
  SetDivers(std::move(list));
}

void DiveDetailsViewModel::SetDivers(QVector<DiveDetailsDiver> divers) {
  SetDiversToDisplay(std::move(divers));
}

void DiveDetailsViewModel::SetDiversToDisplay(QVector<DiveDetailsDiver> divers) {
  beginRemoveRows({}, 0, rowCount() - 1);
  endRemoveRows();

  m_divers = std::move(divers);

  beginInsertRows({}, 0, m_divers.size() - 1);
  endInsertRows();
}

void DiveDetailsViewModel::Clear() {
  SetDivers({});
}

std::optional<DiveDetailsDiver> DiveDetailsViewModel::GetDiverAtIndex(QModelIndex index) const {
  if (!index.isValid()) {
    return {};
  }
  return m_divers.at(index.row());
}

int DiveDetailsViewModel::rowCount(const QModelIndex &parent) const {
  std::ignore = parent;
  return m_divers.size();
}

int DiveDetailsViewModel::columnCount(const QModelIndex & /*parent*/) const {
  return kColumnsHeaders.size();
}

QVariant DiveDetailsViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
      return section == ColumnId::kDiveType ? int(Qt::AlignLeft | Qt::AlignVCenter)
                                            : int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::FontRole: {
      static QFont default_font{};
      default_font.setBold(true);
      return default_font;
      break;
    }
  }
  return QVariant();
}

QVariant DiveDetailsViewModel::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called

  static const DiveDetailsDiver kEmptyDiver{};
  const auto &diver{row < m_divers.size() ? m_divers.at(row) : kEmptyDiver};

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
      return {};
    case Qt::TextAlignmentRole:
      return col == ColumnId::kDiveType ? int(Qt::AlignLeft | Qt::AlignVCenter)
                                        : int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::CheckStateRole:
      // if (row == 1 && col == 0)  // add a checkbox to cell(1,0)
      //   return Qt::Checked;
      break;
  }
  return QVariant();
}

const QString &DiveDetailsViewModel::GetDivingTypeText(int site_id) const {
  if (const auto kIt{m_db_diving_types.find(site_id)}; kIt != m_db_diving_types.end()) {
    return kIt->second.type_name;
  }

  SPDLOG_WARN("Failed to find diving type with id <{}>", site_id);
  static const QString kUnknownString{tr("INCONNU")};
  return kUnknownString;
}

QString DiveDetailsViewModel::GetDisplayTextForIndex(const DiveDetailsDiver &diver, int col) const {
  switch (col) {
    case ColumnId::kFirstName: {
      return diver.first_name;
    }
    case ColumnId::kLastName: {
      return diver.last_name;
    }
    case ColumnId::kDiveType: {
      return GetDivingTypeText(diver.member.diving_type_id);
    }
    default:
      SPDLOG_WARN("Unknown column title index: <{}>", col);
      break;
  }
  return tr("ERREUR - inconnu");
}

}  // namespace cpsm

#include "moc_DiveDetailsViewModel.cpp"
