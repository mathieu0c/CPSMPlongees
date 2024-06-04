#pragma once

#include <QHeaderView>
#include <QWidget>

#include <Models/DivesViewModel.hpp>

namespace gui {

namespace Ui {
class DiveSearch;
}

class DiveSearch : public QWidget {
  Q_OBJECT

 public:
  explicit DiveSearch(QWidget *parent = nullptr);
  ~DiveSearch();

  std::optional<cpsm::DisplayDive> GetSelectedDive() const;
  QVector<cpsm::DisplayDive> GetSelectedDives() const;

  void SetSectionResizeMode(QHeaderView::ResizeMode mode);

 public slots:
  void RefreshFromDB(int diver_id = -1);
  void on_tableView_doubleClicked(const QModelIndex &);

 private:
  Ui::DiveSearch *ui;

  cpsm::DivesViewModel m_model;
};

}  // namespace gui
