#pragma once

#include <Models/DivesViewModel.hpp>

namespace cpsm {

class DivesViewModelForDiverEdit : public DivesViewModel {
  Q_OBJECT
  enum ColumnId : int32_t { kDate = 0, kTime = 1, kSite = 2, kType = 3 };

  static constexpr std::array kColumnsHeaders{"Date", "Heure", "Site", "Type"};

 public:
  explicit DivesViewModelForDiverEdit(QObject *parent = nullptr);

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;              /* virtual */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override; /* virtual */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;     /* virtual */

 private:
  QString GetDisplayTextForIndex(const DisplayDive &dive, int col) const;
  QVariant GetBackgroundForIndex(const DisplayDive &dive, int col) const;
};

}  // namespace cpsm
