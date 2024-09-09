#pragma once

#include <QObject>
#include <set>

#include <Models/DiversViewModel.hpp>

namespace cpsm {

class DiveEditNonMembers : public DiversViewModel {
  Q_OBJECT
  enum ColumnId : int32_t { kLastName = 0, kFirstName = 1, kLevel = 2, kBalance = 3 };

  static constexpr std::array kColumnsHeaders{"Nom", "Prénom", "Niveau", "Plongées dûes"};

 public:
  explicit DiveEditNonMembers(QObject *parent = nullptr);

  void LoadFromDB() override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

 private:
  QString GetDisplayTextForIndex(const DiverWithDiveCount &complete_diver, int col) const;
  QVariant GetBackgroundForIndex(const DiverWithDiveCount &complete_diver, int col) const;
};

}  // namespace cpsm
