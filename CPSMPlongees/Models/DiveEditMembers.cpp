#include "DiveEditMembers.hpp"

#include <RawStructs.hpp>

#include "Constants.hpp"

namespace {
struct TmpDBReadDiver {
  cpsm::db::Diver diver{};
  cpsm::db::DiveMember member{};
};
}  // namespace

namespace cpsm {

DiveEditMembers::DiveEditMembers(QObject *parent) : DiveEditNonMembers{parent} {}

void DiveEditMembers::LoadFromDB() {
  static QVector<db::DivingType> diving_type_list{};
  if (diving_type_list.size() == 0) {
    SPDLOG_ERROR("Failed to load diving types from db. Retrying... (Or first attempt)");
    diving_type_list = ::db::readLFromDB<db::DivingType>(
        ::db::Def(), db::ExtractDivingType, "SELECT * FROM %0", {db::DivingType::db_table}, {});
    for (const auto &diving_type : diving_type_list) {
      m_db_diving_types[diving_type.diving_type_id] = diving_type;
    }
    emit DivingTypesUpdated(m_db_diving_types);
  }

  auto list{::db::readLFromDB<TmpDBReadDiver>(
      ::db::Def(),
      [&](const QSqlQuery &query) {
        //
        auto diver{db::ExtractDiver(query)};
        auto dive_member{db::ExtractDiveMember(query)};
        return TmpDBReadDiver{diver, dive_member};
      },
      "SELECT %0.*, %1.* FROM %0 LEFT JOIN "
      "%1 ON %0.%2 = %1.%2 "
      "WHERE %3=? GROUP BY %0.%2;",
      {db::Diver::db_table, db::DiveMember::db_table, db::Diver::diver_id_col, db::DiveMember::dive_id_col},
      {m_dive_id})};

  QVector<DiverWithDiveCount> diver_list{};
  std::map<int, db::DiveMember> members_list{};

  for (const auto &tmp : list) {
    members_list[tmp.diver.diver_id] = tmp.member;
    diver_list.emplace_back(DiverWithDiveCount{tmp.diver, 0});
  }

  SetDivingMembers(std::move(members_list));
  SetDivers(std::move(diver_list));

  emit DiveMembersChanged();
}

QVariant DiveEditMembers::headerData(int section, Qt::Orientation orientation, int role) const {
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
      return section == ColumnId::kDivingType ? int(Qt::AlignLeft | Qt::AlignVCenter)
                                              : int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::FontRole: {
      static QFont default_font{};
      return default_font;
      break;
    }
  }
  return QVariant();
}

QVariant DiveEditMembers::data(const QModelIndex &index, int role) const {
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called

  static const DiverWithDiveCount kEmptyDiver{};
  static const db::DiveMember kEmptyDiveMember{};
  const auto &diver{row < DiversToDisplay().size() ? DiversToDisplay().at(row) : kEmptyDiver};
  const auto &dive_member{std::find_if(GetDivingMembers().cbegin(),
                                       GetDivingMembers().cend(),
                                       [&](const auto &pair) { return pair.first == diver.diver.diver_id; }) !=
                                  GetDivingMembers().cend()
                              ? GetDivingMembers().at(diver.diver.diver_id)
                              : kEmptyDiveMember};

  switch (role) {
    case Qt::DisplayRole:
      return GetDisplayTextForIndex(diver, dive_member, col);
    case Qt::EditRole:
      if (col == ColumnId::kDivingType) {
        return GetDisplayTextForIndex(diver, dive_member, col);
      }
    case Qt::FontRole:
      // if (row == 0 && col == 0) {  // change font only for cell(0,0)
      //     QFont boldFont;
      //     boldFont.setBold(true);
      //     return boldFont;
      // }
      break;
    case Qt::BackgroundRole:
      return GetBackgroundForIndex(diver, dive_member, col);
    case Qt::TextAlignmentRole:
      return col == ColumnId::kDivingType ? int(Qt::AlignLeft | Qt::AlignVCenter)
                                          : int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::CheckStateRole:
      // if (row == 1 && col == 0)  // add a checkbox to cell(1,0)
      //   return Qt::Checked;
      break;
  }
  return QVariant();
}

bool DiveEditMembers::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role == Qt::EditRole && index.column() == ColumnId::kDivingType) {
    int row = index.row();
    if (row < static_cast<int>(GetDivingMembers().size())) {
      m_diving_members[GetDisplayDivers()[row].diver.diver_id].diving_type_id = value.toInt();
      emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
      return true;
    }
  }
  return false;
}

Qt::ItemFlags DiveEditMembers::flags(const QModelIndex &index) const {
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  if (index.column() == ColumnId::kDivingType) {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

int DiveEditMembers::columnCount(const QModelIndex & /*parent*/) const {
  return kColumnsHeaders.size();
}

void DiveEditMembers::SetDiveId(int dive_id) {
  m_dive_id = dive_id;
}

std::set<int> DiveEditMembers::GetMembersDiversIds() const {
  std::set<int> ids{};
  for (const auto &diver : GetDivingMembers()) {
    ids.insert(diver.first);
  }
  return ids;
}

void DiveEditMembers::SetDivers(QVector<DiverWithDiveCount> divers) {
  std::map<int, db::DiveMember> members_list{};
  for (const auto &diver : divers) {
    auto diving_type_id{m_db_diving_types.empty() ? 0 : m_default_diving_type_id};

    if (const auto kIter{std::find_if(
            m_diving_members.cbegin(),
            m_diving_members.cend(),
            [&diver](const auto &id_and_member) { return id_and_member.second.diver_id == diver.diver.diver_id; })};
        kIter != m_diving_members.end()) {
      diving_type_id = kIter->second.diving_type_id;
    }

    members_list[diver.diver.diver_id] = db::DiveMember{m_dive_id, diver.diver.diver_id, diving_type_id};
  }

  SetDivingMembers(std::move(members_list));
  std::sort(divers.begin(), divers.end(), [](const DiverWithDiveCount &lhs, const DiverWithDiveCount &rhs) {
    return lhs.diver.last_name < rhs.diver.last_name;
  });
  DiversViewModel::SetDivers(std::move(divers));
}

size_t DiveEditMembers::AddDivers(const QVector<cpsm::DiverWithDiveCount> &divers, bool select_after_adding) {
  auto tmp{GetDivers()};
  size_t added_count{};
  for (const auto &diver : divers) {
    if (std::find_if(tmp.cbegin(), tmp.cend(), [&diver](const DiverWithDiveCount &diver_with_dive_count) {
          return diver_with_dive_count.diver.diver_id == diver.diver.diver_id;
        }) == tmp.cend()) {
      tmp.emplace_back(diver);
      ++added_count;
    }
  }
  SetDivers(std::move(tmp));
  if (select_after_adding) {
    std::set<int> rows_to_be_selected{};
    for (int i{}; i < GetDisplayDivers().size(); ++i) {
      /* If the diver in the display list was just added, add it to the selection */
      if (std::find_if(divers.cbegin(), divers.cend(), [this, &i](const DiverWithDiveCount &diver) {
            return diver.diver.diver_id == GetDisplayDivers()[i].diver.diver_id;
          }) != divers.cend()) {
        rows_to_be_selected.insert(i);
      }
    }
    emit SetSelectedRows(rows_to_be_selected);
  }
  return added_count;
}

size_t DiveEditMembers::RemoveDivers(const QVector<DiverWithDiveCount> &divers) {
  if (divers.empty()) {
    return {};
  }

  std::set<int> to_remove{};
  for (const auto &diver : divers) {
    to_remove.emplace(diver.diver.diver_id);
  }
  return RemoveDivers(to_remove);
}

size_t DiveEditMembers::RemoveDivers(const std::set<int> &diver_ids) {
  auto tmp{GetDivers()};
  size_t removed_count{};
  for (auto it{tmp.begin()}; it != tmp.end();) {
    if (diver_ids.find(it->diver.diver_id) != diver_ids.end()) {
      it = tmp.erase(it);
      ++removed_count;
    } else {
      ++it;
    }
  }

  SetDivers(std::move(tmp));
  return removed_count;
}

QString DiveEditMembers::GetDivingTypeText(int diving_type_id) const {
  auto it{m_db_diving_types.find(diving_type_id)};
  if (it != m_db_diving_types.end()) {
    return it->second.type_name;
  }
  return "?<" + QString::number(diving_type_id) + ">?";
}

QString DiveEditMembers::GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, const db::DiveMember &member,
                                                int col) const {
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
    case ColumnId::kDivingType: {
      return GetDivingTypeText(member.diving_type_id);
    }
    default:
      SPDLOG_WARN("Unknown column title index: <{}>", col);
      break;
  }
  return tr("ERREUR - inconnu");
}

QVariant DiveEditMembers::GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, const db::DiveMember &member,
                                                int col) const {
  std::ignore = complete_diver;
  switch (col) {
    case ColumnId::kDivingType: {
      return ::consts::colors::GetColorForDivingType(member.diving_type_id);
    }
    default:
      break;
  }
  return {};
}

void DiveEditMembers::SetDivingMembers(std::map<int, db::DiveMember> members) {
  m_diving_members = std::move(members);
  emit DiveMembersChanged();
}

}  // namespace cpsm

#include "moc_DiveEditMembers.cpp"
