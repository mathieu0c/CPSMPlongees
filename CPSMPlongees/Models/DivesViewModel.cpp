#include "DivesViewModel.hpp"

#include <CPSMGlobals.hpp>
#include <Constants.hpp>
#include <QFont>
#include <Utils.hpp>

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

void DivesViewModel::LoadFromDB(int diver_id) {
  m_diving_sites.clear();

  const auto kDivingSitesList{::db::readLFromDB<db::DivingSite>(
      ::db::Def(), db::ExtractDivingSite, "SELECT * FROM %0", {db::DivingSite::db_table}, {})};

  if (kDivingSitesList.size() == 0) {
    SPDLOG_ERROR("Failed to load levels from db. Retrying... (Or first attempt)");
  }
  for (const auto &site : kDivingSitesList) {
    m_diving_sites[site.diving_site_id] = site;
  }

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

  QString base_query{};
  QStringList str_args{
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
      db::DivingType::diving_type_id_col, /* 10 */
      db::Dive::datetime_col              /* 11 */
  };
  QVariantList args{};

  if (diver_id < 0) {
    base_query =
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
        "GROUP BY %4.%6 "
        "ORDER BY %4.%11 DESC;";
  } else {
    base_query =
        "SELECT "
        "     *,"
        "     %0.%1 AS dive_types,"
        "     COUNT(%2.%3) AS diver_count "
        "FROM %4"
        "     LEFT JOIN "
        "         %7 ON %4.%5 = %7.%8"
        "     LEFT JOIN "
        "         %2 ON %4.%6 = %2.%3"
        "     LEFT JOIN "
        "         %0 ON %2.%9 = %0.%10 "
        "GROUP BY %4.%6 "
        "HAVING COUNT(DISTINCT CASE WHEN %2.%12 = ? THEN 1 END) > 0 "
        "ORDER BY %4.%11 DESC;";
    str_args.append(db::DiveMember::diver_id_col); /* 12 */

    args.append(diver_id);
  }

  auto list{::db::readLFromDB<DisplayDive>(
      ::db::Def(),
      [&](const QSqlQuery &query) {
        auto dive{db::ExtractDive(query)};
        auto diver_count{query.value("diver_count").toInt()};
        auto diving_types{query.value("dive_types").toString()};
        return DisplayDive{.dive = std::move(dive), .diver_count = diver_count, .dive_types = std::move(diving_types)};
      },
      base_query,
      str_args,
      args)};

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
      return section == ColumnId::kDiverCount ? int(Qt::AlignLeft | Qt::AlignVCenter)
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
      return col == ColumnId::kDiverCount ? int(Qt::AlignLeft | Qt::AlignVCenter)
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

void DivesViewModel::SetDateFilter(const QDate &start, const QDate &end) {
  m_filters[Filters::kFilterDate].filter = [start, end](const DisplayDive &dive) {
    return dive.dive.datetime.date() >= start && dive.dive.datetime.date() <= end;
  };
  m_filters[Filters::kFilterDate].active = true;
  ReapplyFilters();
}

void DivesViewModel::SetTypeFilter(const QString &type_str, bool active) {
  const auto kOldActive{m_filters[Filters::kFilterType].active};
  m_filters[Filters::kFilterType].active = active;
  m_filters[Filters::kFilterType].filter = [type_str](const DisplayDive &dive) {
    return dive.dive_types.contains(type_str);
  };

  if (!active && kOldActive == active) {
    return;
  }

  ReapplyFilters();
}

void DivesViewModel::SetDiverCountFilter(std::function<bool(int, int)> comparison_operator, int value, bool active) {
  // const auto kOldActive{m_filters[Filters::kFilterType].active};
  m_filters[Filters::kFilterType].active = active;
  m_filters[Filters::kFilterType].filter = [comparison_operator, value](const DisplayDive &dive) {
    return comparison_operator(dive.diver_count, value);
  };

  // if (!active && kOldActive == active) {
  //   return;
  // }

  ReapplyFilters();
}

const QString &DivesViewModel::GetDivingSiteText(int site_id) const {
  if (const auto kIt{m_diving_sites.find(site_id)}; kIt != m_diving_sites.end()) {
    return kIt->second.site_name;
  }

  SPDLOG_WARN("Failed to find diving site with id <{}>", site_id);
  static const QString kUnknownString{tr("INCONNU")};
  return kUnknownString;
}

void DivesViewModel::InitFilters() {
  /* We define the maxFilterValue filter just to avoid crash by mistake... */
  m_filters[Filters::kMaxFilterEnumValue] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &) { return true; }, .active = false, .negate = false};

  /* Actual filters */
  m_filters[Filters::kFilterMorning] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &dive) { return IsMorning(dive.dive.datetime); },
                                 .active = false,
                                 .negate = false};

  m_filters[Filters::kFilterAfternoon] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &dive) { return !IsMorning(dive.dive.datetime); },
                                 .active = false,
                                 .negate = false};

  m_filters[Filters::kFilterDate] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &) { return true; }, .active = false, .negate = false};

  m_filters[Filters::kFilterType] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &) { return true; }, .active = false, .negate = false};

  m_filters[Filters::kFilterDiverCount] =
      utils::Filter<DisplayDive>{.filter = [](const DisplayDive &) { return true; }, .active = false, .negate = false};

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
      return dive.dive.datetime.time().toString(cpsm::consts::kTimeFormat);
    }
    case ColumnId::kSite: {
      return GetDivingSiteText(dive.dive.diving_site_id);
    }
    case ColumnId::kDiverCount: {
      return QString::number(dive.diver_count);
    }
    // case ColumnId::kDiveType: {
    //   return dive.dive_types;
    // }
    default:
      SPDLOG_WARN("Unknown column title index: <{}>", col);
      break;
  }
  return tr("ERREUR - inconnu");
}

QVariant DivesViewModel::GetBackgroundForIndex(const DisplayDive &complete_dive, int col) const {
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

#include "moc_DivesViewModel.cpp"
