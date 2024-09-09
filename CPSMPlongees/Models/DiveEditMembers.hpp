#pragma once

#include <QObject>
#include <set>

#include <Models/DiveEditNonMembers.hpp>

namespace cpsm {

class DiveEditMembers : public DiveEditNonMembers {
  Q_OBJECT

  static constexpr std::array kColumnsHeaders{"Nom", "Prénom", "Niveau", "Type de plongée"};

 public:
  static constexpr int m_default_diving_type_id{
      std::numeric_limits<int>::min()}; /* Should be made const... Cannot copy qobject anyway */

  enum ColumnId : int32_t { kLastName = 0, kFirstName = 1, kLevel = 2, kDivingType = 3 };

 signals:
  void DiveMembersChanged();
  void DivingTypesUpdated(const std::map<int, db::DivingType> &);
  void SetSelectedRows(const std::set<int> &);

 public:
  explicit DiveEditMembers(QObject *parent = nullptr);

  void LoadFromDB() override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  void SetDiveId(int dive_id); /* LoadFromDB should be called after this function to refresh info */

  std::set<int> GetMembersDiversIds() const;
  const auto &GetDivingMembers() const {
    return m_diving_members;
  }

  void SetDivers(QVector<DiverWithDiveCount> divers) override;
  size_t AddDivers(const QVector<DiverWithDiveCount> &divers, bool select_after_adding);
  size_t RemoveDivers(const QVector<DiverWithDiveCount> &divers);
  size_t RemoveDivers(const std::set<int> &diver_ids);

 protected:
  QString GetDivingTypeText(int diving_type_id) const;

 private:
  QString GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, const db::DiveMember &member, int col) const;
  QVariant GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, const db::DiveMember &member, int col) const;

  void SetDivingMembers(std::map<int, db::DiveMember> members);

 private:
  std::map<int, db::DivingType> m_db_diving_types{};

  int m_dive_id{-1};
  std::map<int, db::DiveMember> m_diving_members{}; /* diver_id, diving_members */
};

}  // namespace cpsm
