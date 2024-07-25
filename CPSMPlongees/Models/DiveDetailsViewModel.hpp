#pragma once

#include <QAbstractTableModel>
#include <RawStructs.hpp>

#include <Utils/list_filters.hpp>

namespace cpsm {

struct DiveDetailsDiver {
  db::DiveMember member;
  QString first_name;
  QString last_name;
};

class DiveDetailsViewModel : public QAbstractTableModel {
  Q_OBJECT
  enum ColumnId : int32_t { kLastName = 0, kFirstName = 1, kDiveType = 2 };

  static constexpr std::array kColumnsHeaders{"Nom", "Prénom", "Type"};

 public:
  explicit DiveDetailsViewModel(QObject *parent = nullptr);

  void LoadFromDB(int dive_id);

  void SetDivers(QVector<DiveDetailsDiver> divers);
  void SetDiversToDisplay(QVector<DiveDetailsDiver> divers);
  void Clear();

  std::optional<DiveDetailsDiver> GetDiverAtIndex(QModelIndex index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  const QString &GetDivingTypeText(int site_id) const;

 private:
  QString GetDisplayTextForIndex(const DiveDetailsDiver &diver, int col) const;

 private:
  std::map<int, db::DivingType> m_db_diving_types{};

  QVector<DiveDetailsDiver> m_divers;
};

}  // namespace cpsm
