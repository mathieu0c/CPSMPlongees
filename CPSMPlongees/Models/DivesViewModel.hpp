#pragma once

#include <QAbstractTableModel>
#include <RawStructs.hpp>

#include <Utils/list_filters.hpp>

namespace cpsm {

struct DisplayDive {
  db::Dive dive{};
  int32_t diver_count{};
  QString dive_types{};
};

class DivesViewModel : public QAbstractTableModel {
  Q_OBJECT
  enum ColumnId : int32_t { kDate = 0, kTime = 1, kSite = 2, kDiverCount = 3 };

  static constexpr std::array kColumnsHeaders{"Date", "Heure", "Site", "Plongeurs"};

 public:
  enum Filters : int32_t {
    kFilterMorning,
    kFilterAfternoon,
    kFilterDate,
    kFilterType,
    kFilterDiverCount,
    kMaxFilterEnumValue
  };

 public:
  explicit DivesViewModel(QObject *parent = nullptr);

  virtual void LoadFromDB(int diver_id); /* virtual */

  virtual void SetDives(QVector<DisplayDive> dives);          /* virtual */
  virtual void SetDivesToDisplay(QVector<DisplayDive> dives); /* virtual */

  std::optional<DisplayDive> GetDiveAtIndex(QModelIndex index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;                 /* virtual */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;              /* virtual */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override; /* virtual */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;     /* virtual */

  void SetFilterEnabled(Filters filter, bool enabled);
  void SetFilterNegate(Filters filter, bool negate);

  void SetDateFilter(const QDate &start, const QDate &end);
  void SetTypeFilter(const QString &type_str, bool active);
  void SetDiverCountFilter(std::function<bool(int, int)> comparison_operator, int value, bool active);

  const QString &GetDivingSiteText(int site_id) const;

  const auto &GetDisplayDives() const {
    return m_dives_to_display;
  }

 private:
  void InitFilters();

  void ReapplyFilters();

  QString GetDisplayTextForIndex(const DisplayDive &dive, int col) const;
  QVariant GetBackgroundForIndex(const DisplayDive &dive, int col) const;

 private:
  std::map<int, db::DiverLevel> m_db_levels{};

  QVector<DisplayDive> m_dives;
  QVector<DisplayDive> m_dives_to_display;

  std::map<int, db::DivingSite> m_diving_sites{};

  std::map<Filters, utils::Filter<DisplayDive>> m_filters{};
};

}  // namespace cpsm
