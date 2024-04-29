#pragma once

#include <QAbstractTableModel>
#include <RawStructs.hpp>

class MyModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit MyModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

namespace cpsm {

class DiverTableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit DiverTableModel(QObject *parent = nullptr);

  void LoadFromDB();

  void SetDivers(QVector<db::Diver> divers);
  void SetDiversToDisplay(QVector<db::Diver> divers);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

 private:
  QVector<db::Diver> m_divers;
  QVector<db::Diver> m_divers_to_display;
};

}  // namespace cpsm
