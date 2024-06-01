#include "DivesViewModel.hpp"

#include <CPSMGlobals.hpp>
#include <Constants.hpp>
#include <QFont>

namespace {

bool IsMorning(const QDateTime &dt) {
  return dt.time().hour() < 12;
}

}  // namespace

namespace cpsm {

DivesViewModel::DivesViewModel(QObject *parent) : QAbstractTableModel(parent) {
  // const auto kLevelList{::db::readLFromDB<db::DiveLevel>(
  //     ::db::Def(), db::ExtractDiveLevel, "SELECT * FROM %0", {db::DiveLevel::db_table}, {})};

  // for (const auto &level : kLevelList) {
  //   m_db_levels[level.dive_level_id] = level;
  // }

  InitFilters();

  // if (db::Def().isOpen()) {
  //   LoadFromDB();
  // }
}

void DivesViewModel::LoadFromDB() {
  // static QVector<db::DiveLevel> level_list{};
  // if (level_list.size() == 0) {
  //   SPDLOG_ERROR("Failed to load levels from db. Retrying... (Or first attempt)");
  //   level_list = ::db::readLFromDB<db::DiveLevel>(
  //       ::db::Def(), db::ExtractDiveLevel, "SELECT * FROM %0", {db::DiveLevel::db_table}, {});
  //   for (const auto &level : level_list) {
  //     m_db_levels[level.dive_level_id] = level;
  //   }
  // }

  SPDLOG_TRACE("Reloading DivesViewModel from db");

  // SELECT
  //     *,
  //     GROUP_CONCAT(DISTINCT DivingTypes.type_name) AS dive_types,
  //     COUNT(DivesMembers.diver_id) AS diver_count
  //         FROM
  //             Dives
  //                 LEFT JOIN
  //                     DivingSites ON Dives.diving_site_id = DivingSites.diving_site_id
  //           LEFT JOIN
  //               DivesMembers ON Dives.dive_id = DivesMembers.dive_id
  //           LEFT JOIN
  //               DivingTypes ON DivesMembers.diving_type_id = DivingTypes.diving_type_id
  //           GROUP BY
  //               Dives.dive_id;

  auto list{::db::readLFromDB<DisplayDive>(
      ::db::Def(),
      [&](const QSqlQuery &query) {
        auto dive{db::ExtractDive(query)};
        auto diver_count{query.value("diver_count").toInt()};
        auto diving_types{query.value("dive_types").toString()};
        return DisplayDive{.dive = std::move(dive), .diver_count = diver_count, .dive_types = std::move(diving_types)};
      },
      "SELECT "
      "     *,"
      "     GROUP_CONCAT(DISTINCT %0.%1) AS dive_types,"
      "     COUNT(%2.%3) AS diver_count "
      "FROM %4"
      "     LEFT JOIN "
      "         %7 ON %4.%5 = %7.%8"
      "     LEFT JOIN "
      "         %2 ON %4.%6 = %2.%3"
      "     LEFT JOIN "
      "         %0 ON %2.%9 = %0.%10 "
      "GROUP BY %4.%6;",
      {
          db::DivingType::db_table,           /* 0 */
          db::DivingType::type_name_col,      /* 1 */
          db::DiveMember::db_table,           /* 2 */
          db::DiveMember::dive_id_col,        /* 3 */
          db::Dive::db_table,                 /* 4 */
          db::Dive::diving_site_id_col,       /* 5 */
          db::Dive::dive_id_col,              /* 6 */
          db::DivingSite::db_table,           /* 7 */
          db::DivingSite::diving_site_id_col, /* 8 */
          db::DiveMember::diving_type_id_col, /* 9 */
          db::DivingType::diving_type_id_col  /* 10 */
      },
      {})};

  SetDives(std::move(list));
}

void DivesViewModel::SetDives(QVector<DisplayDive> dives) {
  m_dives = std::move(dives);

  ReapplyFilters();
}

void DivesViewModel::SetDivesToDisplay(QVector<DisplayDive> dives) {
  beginRemoveRows({}, 0, rowCount() - 1);
  endRemoveRows();

  m_dives_to_display = std::move(dives);

  beginInsertRows({}, 0, m_dives_to_display.size() - 1);
  endInsertRows();
}

std::optional<DisplayDive> DivesViewModel::GetDiveAtIndex(QModelIndex index) const {
  if (!index.isValid()) {
    return {};
  }
  return m_dives_to_display.at(index.row());
}

int DivesViewModel::rowCount(const QModelIndex &parent) const {
  std::ignore = parent;
  return m_dives_to_display.size();
}

int DivesViewModel::columnCount(const QModelIndex & /*parent*/) const {
  return kColumnsHeaders.size();
}

QVariant DivesViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
      return default_font;
      break;
    }
  }
  return QVariant();
}

QVariant DivesViewModel::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called

  static const DisplayDive kEmptyDive{};
  const auto &dive{row < m_dives_to_display.size() ? m_dives_to_display.at(row) : kEmptyDive};

  switch (role) {
    case Qt::DisplayRole:
      return GetDisplayTextForIndex(dive, col);
    case Qt::FontRole:
      // if (row == 0 && col == 0) {  // change font only for cell(0,0)
      //     QFont boldFont;
      //     boldFont.setBold(true);
      //     return boldFont;
      // }
      break;
    case Qt::BackgroundRole:
      return GetBackgroundForIndex(dive, col);
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

void DivesViewModel::SetFilterEnabled(Filters filter, bool enabled) {
  m_filters[filter].active = enabled;
  ReapplyFilters();
}

void DivesViewModel::SetFilterNegate(Filters filter, bool negate) {
  m_filters[filter].negate = negate;
  ReapplyFilters();
}

/*Enable the name filter if name isn't empty. Search in bot first and last name*/
void DivesViewModel::SetDateFilter(const QDate &start, const QDate &end) {
  std::ignore = start;
  std::ignore = end;
  // if (name.isEmpty()) {
  //   m_filters[Filters::kFilterName].active = false;
  //   ReapplyFilters();
  //   return;
  // }

  // m_filters[Filters::kFilterName].filter = [name](const DisplayDive &dive) { return ::SearchString(name, dive);
  // }; m_filters[Filters::kFilterName].active = true; ReapplyFilters();
}

void DivesViewModel::InitFilters() {
  /* We define the maxFilterValue filter just to avoid crash by mistake... */
  m_filters[Filters::kMaxFilterEnumValue] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &) { return true; }, .active = false, .negate = false};

  /* Actual filters */
  m_filters[Filters::kFilterMorning] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &dive) { return ::IsMorning(dive.dive.datetime); },
                                 .active = false,
                                 .negate = false};

  m_filters[Filters::kFilterAfternoon] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &dive) { return !::IsMorning(dive.dive.datetime); },
                                 .active = false,
                                 .negate = false};

  // m_filters[Filters::kFilterIsMember] = utils::Filter<DisplayDive>{
  //     .filter = [](const DisplayDive &dive) { return db::IsDiveCurrentlyAMember(dive.dive); },
  //     .active = false,
  //     .negate = false};

  // m_filters[Filters::kFilterIsCurrentlyRegistered] = utils::Filter<DisplayDive>{
  //     .filter = [](const DisplayDive &dive) { return db::IsDiveCurrentlyRegistered(dive.dive); },
  //     .active = false,
  //     .negate = false};

  // m_filters[Filters::kFilterPositiveBalance] = utils::Filter<DisplayDive>{
  //     .filter = [](const DisplayDive &dive) { return dive.Balance() > 0; }, .active = false, .negate = false};

  // m_filters[Filters::kFilterNegativeBalance] = utils::Filter<DisplayDive>{
  //     .filter = [](const DisplayDive &dive) { return dive.Balance() < 0; }, .active = false, .negate = false};

  /* Check we didn't forget anything */

  if (m_filters.size() != Filters::kMaxFilterEnumValue + 1) {
    SPDLOG_ERROR("Filters not initialized correctly. Expected <{}> filters, got <{}>",
                 Filters::kMaxFilterEnumValue + 1,
                 m_filters.size());
    throw std::runtime_error("DivesViewModel filters list not initialized correctly");
  }
}

void DivesViewModel::ReapplyFilters() {
  SPDLOG_DEBUG("Applying filters to dives list...");
  auto to_display{m_dives};
  utils::FilterList(&to_display, m_filters);

  SetDivesToDisplay(std::move(to_display));
}

QString DivesViewModel::GetDisplayTextForIndex(const DisplayDive &dive, int col) const {
  switch (col) {
    case ColumnId::kDate: {
      return dive.dive.datetime.date().toString(cpsm::consts::kDateUserFormat);
    }
    case ColumnId::kTime: {
      return dive.dive.datetime.time().toString();
    }
    case ColumnId::kSite: {
      return "TMP SITE";
    }
    case ColumnId::kDiverCount: {
      return QString::number(dive.diver_count);
    }
    case ColumnId::kDiveType: {
      return dive.dive_types;
    }
    default:
      SPDLOG_WARN("Unknown column title index: <{}>", col);
      break;
  }
  return tr("ERREUR - inconnu");
}

QVariant DivesViewModel::GetBackgroundForIndex(const DisplayDive &complete_dive, int col) const {
  const auto &dive{complete_dive.dive};
  switch (col) {
    case ColumnId::kDate:
    case ColumnId::kTime: {
      return IsMorning(dive.datetime) ? ::consts::colors::kBackgroundBlue : ::consts::colors::kBackgroundYellow;
    }
    default:
      break;
  }
  return {};
}

}  // namespace cpsm

#include "moc_DivesViewModel.cpp"
