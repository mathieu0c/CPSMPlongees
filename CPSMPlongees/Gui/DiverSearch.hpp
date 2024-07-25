#pragma once

#include <QHeaderView>
#include <QWidget>

#include <Models/DiversViewModel.hpp>

namespace gui {

namespace Ui {
class DiverSearch;
}

class DiverSearch : public QWidget {
  Q_OBJECT

 signals:
  void DoubleClickOnDiver(cpsm::DiverWithDiveCount diver);

 public:
  explicit DiverSearch(cpsm::DiversViewModel *model, QWidget *parent = nullptr);
  explicit DiverSearch(QWidget *parent = nullptr);
  ~DiverSearch();

  std::optional<cpsm::DiverWithDiveCount> GetSelectedDiver() const;
  QVector<cpsm::DiverWithDiveCount> GetSelectedDivers() const;

  void SetSectionResizeMode(QHeaderView::ResizeMode mode);
  void SetItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);

  void SetModel(cpsm::DiversViewModel *model);

 public slots:
  void RefreshFromDB();

  void SetSelectedRows(const std::set<int> &rows);

 private slots:
  void on_tableView_doubleClicked(const QModelIndex &index);

 private:
  Ui::DiverSearch *ui;

  cpsm::DiversViewModel *m_model{nullptr};
};

}  // namespace gui
