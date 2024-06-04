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
  explicit DiverSearch(QWidget *parent = nullptr);
  ~DiverSearch();

  std::optional<cpsm::DiverWithDiveCount> GetSelectedDiver() const;
  QVector<cpsm::DiverWithDiveCount> GetSelectedDivers() const;

  void SetSectionResizeMode(QHeaderView::ResizeMode mode);

 public slots:
  void RefreshFromDB();

 private slots:
  void on_tableView_doubleClicked(const QModelIndex &index);

 private:
  Ui::DiverSearch *ui;

  cpsm::DiversViewModel m_model;
};

}  // namespace gui
