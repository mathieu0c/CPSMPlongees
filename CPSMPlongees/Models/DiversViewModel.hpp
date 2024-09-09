#pragma once

#include <QAbstractTableModel>
#include <RawStructs.hpp>
#include <set>

#include <Utils/list_filters.hpp>

namespace cpsm {

struct DiverWithDiveCount {
  db::Diver diver{};
  int dive_count{};
  int dive_count_in_last_season{};

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
    std::sort(divers.begin(), divers.end(), [](const DiverWithDiveCount &lhs, const DiverWithDiveCount &rhs) {
      return lhs.diver.last_name < rhs.diver.last_name;
    });
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
      "Nom", "Prénom", "Dernière inscription", "Adhérent ?", "Certificat médical", "Niveau", "Plongées dûes"};

 public:
  enum Filters : int32_t {
    kFilterName = 0,
    kFilterIsMember = 1,
    kFilterIsCurrentlyRegistered = 2,
    kFilterPositiveBalance = 3,
    kFilterNegativeBalance = 4,
    kFilterHideDiversIds = 5,
    kMaxFilterEnumValue
  };

 public:
  explicit DiversViewModel(QObject *parent = nullptr);

  virtual void LoadFromDB(); /* virtual */

  virtual void SetDivers(QVector<DiverWithDiveCount> divers);          /* virtual */
  virtual void SetDiversToDisplay(QVector<DiverWithDiveCount> divers); /* virtual */

  std::optional<DiverWithDiveCount> GetDiverAtIndex(QModelIndex index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;              /* virtual */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override; /* virtual */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;     /* virtual */

  void SetFilterEnabled(Filters filter, bool enabled);
  void SetFilterNegate(Filters filter, bool negate);
  /* Enable the name filter if name isn't empty. Search in bot first and last name */
  void SetNameFilter(const QString &name);
  void SetHideDiversIdsFilter(std::set<int> divers_ids);

 private:
  void InitFilters();

  void ReapplyFilters();

  QString GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, int col) const;
  QVariant GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, int col) const;

 protected:
  QString GetLevelText(int level_id) const;
  const auto &GetDivers() const {
    return m_divers;
  }
  const auto &GetDisplayDivers() const {
    return DiversToDisplay();
  }

 private:
  std::map<int, db::DiverLevel> m_db_levels{};

  QVector<DiverWithDiveCount> m_divers;

  std::map<Filters, utils::Filter<DiverWithDiveCount>> m_filters{};
};

}  // namespace cpsm
