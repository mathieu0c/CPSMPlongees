#pragma once

#include <QAbstractTableModel>
#include <RawStructs.hpp>

#include <Utils/list_filters.hpp>

namespace cpsm {

struct DiverWithDiveCount {
  db::Diver diver{};
  int dive_count{};

  auto Balance() const {
    return diver.paid_dives - dive_count;
  }
};

class DiverDisplayListOwner {
 public:
  const auto &DiversToDisplay() const {
    return m_divers_to_display;
  }

  void SetDiversToDisplay(QVector<DiverWithDiveCount> divers) {
    m_divers_to_display = std::move(divers);
  }

 private:
  QVector<DiverWithDiveCount> m_divers_to_display;
};

class DiversViewModel : public QAbstractTableModel, public DiverDisplayListOwner {
  Q_OBJECT
  enum ColumnId : int32_t {
    kLastName = 0,
    kFirstName = 1,
    kRegistrationDate = 2,
    kMember = 3,
    kValidCertificate = 4,
    kLevel = 5,
    kBalance = 6
  };

  static constexpr std::array kColumnsHeaders{
      "Nom", "Prénom", "Dernière inscription", "Membre ?", "Certificat médical", "Niveau", "Solde (plongées)"};

 public:
  enum Filters : int32_t {
    kFilterName = 0,
    kFilterIsMember = 1,
    kFilterIsCurrentlyRegistered = 2,
    kFilterPositiveBalance = 3,
    kFilterNegativeBalance = 4,
    kMaxFilterEnumValue
  };

 public:
  explicit DiversViewModel(QObject *parent = nullptr);

  void LoadFromDB();

  void SetDivers(QVector<DiverWithDiveCount> divers);
  void SetDiversToDisplay(QVector<DiverWithDiveCount> divers);

  std::optional<DiverWithDiveCount> GetDiverAtIndex(QModelIndex index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void SetFilterEnabled(Filters filter, bool enabled);
  void SetFilterNegate(Filters filter, bool negate);
  /* Enable the name filter if name isn't empty. Search in bot first and last name */
  void SetNameFilter(const QString &name);

 private:
  void InitFilters();

  void ReapplyFilters();

  QString GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, int col) const;
  QVariant GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, int col) const;

 private:
  std::map<int, db::DiverLevel> m_db_levels{};

  QVector<DiverWithDiveCount> m_divers;

  std::map<Filters, utils::Filter<DiverWithDiveCount>> m_filters{};
};

}  // namespace cpsm
