#include "DiversViewModel.hpp"

#include "Constants.hpp"

namespace {

bool SearchString(const QString &search_line, const cpsm::DiverWithDiveCount &search_in) {
  const auto kLineParts{search_line.split(' ', Qt::SkipEmptyParts)};
  int32_t match_count{};
  for (const auto &part : kLineParts) {
    if (search_in.diver.first_name.contains(part, Qt::CaseInsensitive) ||
        search_in.diver.last_name.contains(part, Qt::CaseInsensitive)) {
      match_count++;
    }
  }
  return match_count == kLineParts.size();
}

}  // namespace

namespace cpsm {

DiversViewModel::DiversViewModel(QObject *parent) : QAbstractTableModel(parent) {
  const auto kLevelList{::db::readLFromDB<db::DiverLevel>(
      ::db::Def(), db::ExtractDiverLevel, "SELECT * FROM %0", {db::DiverLevel::db_table}, {})};

  for (const auto &level : kLevelList) {
    m_db_levels[level.diver_level_id] = level;
  }

  InitFilters();

  // if (db::Def().isOpen()) {
  //   LoadFromDB();
  // }
}

void DiversViewModel::LoadFromDB() {
  static QVector<db::DiverLevel> level_list{};
  if (level_list.size() == 0) {
    SPDLOG_ERROR("Failed to load levels from db. Retrying... (Or first attempt)");
    level_list = ::db::readLFromDB<db::DiverLevel>(
        ::db::Def(), db::ExtractDiverLevel, "SELECT * FROM %0", {db::DiverLevel::db_table}, {});
    for (const auto &level : level_list) {
      m_db_levels[level.diver_level_id] = level;
    }
  }

  SPDLOG_TRACE("Reloading DiversViewModel from db");
  auto list{::db::readLFromDB<DiverWithDiveCount>(
      ::db::Def(),
      [&](const QSqlQuery &query) {
        //
        auto diver{db::ExtractDiver(query)};
        auto dive_count{query.value("dive_count").toInt()};
        return DiverWithDiveCount{diver, dive_count};
      },
      "SELECT %0.*, COUNT(%1.%2) AS dive_count FROM %0 LEFT JOIN "
      "%1 ON %0.%3 = %1.%3 GROUP BY %0.%3;",
      {db::Diver::db_table, db::DiveMember::db_table, db::DiveMember::dive_id_col, db::Diver::diver_id_col},
      {})};

  SetDivers(std::move(list));
}

void DiversViewModel::SetDivers(QVector<DiverWithDiveCount> divers) {
  m_divers = std::move(divers);

  ReapplyFilters();
}

void DiversViewModel::SetDiversToDisplay(QVector<DiverWithDiveCount> divers) {
  beginRemoveRows({}, 0, rowCount() - 1);
  endRemoveRows();

  DiverDisplayListOwner::SetDiversToDisplay(std::move(divers));

  beginInsertRows({}, 0, DiversToDisplay().size() - 1);
  endInsertRows();

  // emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, columnCount() - 1));
}

std::optional<DiverWithDiveCount> DiversViewModel::GetDiverAtIndex(QModelIndex index) const {
  if (!index.isValid()) {
    return {};
  }
  return DiversToDisplay().at(index.row());
}

int DiversViewModel::rowCount(const QModelIndex &parent) const {
  std::ignore = parent;
  return DiversToDisplay().size();
}

int DiversViewModel::columnCount(const QModelIndex & /*parent*/) const {
  return kColumnsHeaders.size();
}

QVariant DiversViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QVariant DiversViewModel::data(const QModelIndex &index, int role) const {
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

void DiversViewModel::SetFilterEnabled(Filters filter, bool enabled) {
  m_filters[filter].active = enabled;
  ReapplyFilters();
}

void DiversViewModel::SetFilterNegate(Filters filter, bool negate) {
  m_filters[filter].negate = negate;
  ReapplyFilters();
}

/*Enable the name filter if name isn't empty. Search in bot first and last name*/
void DiversViewModel::SetNameFilter(const QString &name) {
  if (name.isEmpty()) {
    m_filters[Filters::kFilterName].active = false;
    ReapplyFilters();
    return;
  }

  m_filters[Filters::kFilterName].filter = [name](const DiverWithDiveCount &diver) {
    return ::SearchString(name, diver);
  };
  m_filters[Filters::kFilterName].active = true;
  ReapplyFilters();
}

void DiversViewModel::InitFilters() {
  /* We define the maxFilterValue filter just to avoid crash by mistake... */
  m_filters[Filters::kMaxFilterEnumValue] = utils::Filter<DiverWithDiveCount>{
      .filter = [](const DiverWithDiveCount &) { return true; }, .active = false, .negate = false};

  /* Actual filters */
  m_filters[Filters::kFilterName] =
      utils::Filter<DiverWithDiveCount>{/* Default name filter does nothing */
                                        .filter = [](const DiverWithDiveCount &) { return true; },
                                        .active = false,
                                        .negate = false};

  m_filters[Filters::kFilterIsMember] = utils::Filter<DiverWithDiveCount>{
      .filter = [](const DiverWithDiveCount &diver) { return db::IsDiverCurrentlyAMember(diver.diver); },
      .active = false,
      .negate = false};

  m_filters[Filters::kFilterIsCurrentlyRegistered] = utils::Filter<DiverWithDiveCount>{
      .filter = [](const DiverWithDiveCount &diver) { return db::IsDiverCurrentlyRegistered(diver.diver); },
      .active = false,
      .negate = false};

  m_filters[Filters::kFilterPositiveBalance] = utils::Filter<DiverWithDiveCount>{
      .filter = [](const DiverWithDiveCount &diver) { return diver.Balance() > 0; }, .active = false, .negate = false};

  m_filters[Filters::kFilterNegativeBalance] = utils::Filter<DiverWithDiveCount>{
      .filter = [](const DiverWithDiveCount &diver) { return diver.Balance() < 0; }, .active = false, .negate = false};

  /* Check we didn't forget anything */

  if (m_filters.size() != Filters::kMaxFilterEnumValue + 1) {
    SPDLOG_ERROR("Filters not initialized correctly. Expected <{}> filters, got <{}>",
                 Filters::kMaxFilterEnumValue,
                 m_filters.size());
    throw std::runtime_error("DiversViewModel filters list not initialized correctly");
  }
}

void DiversViewModel::ReapplyFilters() {
  auto to_display{m_divers};
  utils::FilterList(&to_display, m_filters);

  SetDiversToDisplay(std::move(to_display));
}

QString DiversViewModel::GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, int col) const {
  auto lambda_bool{[](bool b) { return b ? tr("Oui") : tr("Non"); }};

  const auto &diver{complete_diver.diver};

  switch (col) {
    case ColumnId::kLastName: {
      return diver.last_name;
    }
    case ColumnId::kFirstName: {
      return diver.first_name;
    }
    case ColumnId::kRegistrationDate: {
      return diver.registration_date.toString(cpsm::consts::kDateUserFormat);
    }
    case ColumnId::kMember: {
      return lambda_bool(cpsm::db::IsDiverCurrentlyAMember(diver));
    }
    case ColumnId::kValidCertificate: {
      return diver.certif_date.toString(cpsm::consts::kDateUserFormat);
    }
    case ColumnId::kLevel: {
      auto it{m_db_levels.find(diver.diver_level_id)};
      if (it != m_db_levels.end()) {
        return it->second.level_name;
      }
      return "?<" + QString::number(diver.diver_level_id) + ">?";
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

QVariant DiversViewModel::GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, int col) const {
  auto lambda_bool{[](bool b) { return b ? ::consts::colors::kBackgroundGreen : ::consts::colors::kBackgroundRed; }};

  const auto &diver{complete_diver.diver};
  switch (col) {
    case ColumnId::kRegistrationDate: {
      return lambda_bool(db::IsDiverCurrentlyRegistered(diver));
    }
    case ColumnId::kMember: {
      return lambda_bool(cpsm::db::IsDiverCurrentlyAMember(diver));
    }
    case ColumnId::kValidCertificate: {
      return lambda_bool(db::IsDiverMedicalCertificateValid(diver));
    }
    case ColumnId::kBalance: {
      return lambda_bool(complete_diver.Balance() >= 0);
    }
    default:
      break;
  }
  return {};
}

}  // namespace cpsm

#include "moc_DiversViewModel.cpp"
