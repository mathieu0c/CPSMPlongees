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
  enum ColumnId : int32_t { kDate = 0, kTime = 1, kSite = 2, kDiverCount = 3, kDiveType = 4 };

  static constexpr std::array kColumnsHeaders{"Date", "Heure", "Site", "Plongeurs", "Types de plongées"};

 public:
  enum Filters : int32_t { kFilterMorning, kFilterAfternoon, kMaxFilterEnumValue };

 public:
  explicit DivesViewModel(QObject *parent = nullptr);

  void LoadFromDB();

  void SetDives(QVector<DisplayDive> dives);
  void SetDivesToDisplay(QVector<DisplayDive> dives);

  std::optional<DisplayDive> GetDiveAtIndex(QModelIndex index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void SetFilterEnabled(Filters filter, bool enabled);
  void SetFilterNegate(Filters filter, bool negate);
  /* Enable the name filter if name isn't empty. Search in bot first and last name */
  void SetDateFilter(const QDate &start, const QDate &end);

 private:
  void InitFilters();

  void ReapplyFilters();

  QString GetDisplayTextForIndex(const DisplayDive &dive, int col) const;
  QVariant GetBackgroundForIndex(const DisplayDive &dive, int col) const;

 private:
  std::map<int, db::DiverLevel> m_db_levels{};

  QVector<DisplayDive> m_dives;
  QVector<DisplayDive> m_dives_to_display;

  std::map<Filters, utils::Filter<DisplayDive>> m_filters{};
};

}  // namespace cpsm
