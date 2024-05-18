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
