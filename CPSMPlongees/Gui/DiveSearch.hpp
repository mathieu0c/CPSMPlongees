#pragma once

#include <QHeaderView>
#include <QWidget>

#include <Models/DivesViewModel.hpp>
#include <set>

namespace gui {

namespace Ui {
class DiveSearch;
}

class DiveSearch : public QWidget {
  Q_OBJECT

 signals:
  void diveSelected(const cpsm::DisplayDive &dive);
  void DoubleClickOnDive(const cpsm::DisplayDive &dive);

 public:
  explicit DiveSearch(QWidget *parent = nullptr);
  ~DiveSearch();

  std::optional<cpsm::DisplayDive> GetSelectedDive() const;
  QVector<cpsm::DisplayDive> GetSelectedDives() const;

  void SetSectionResizeMode(QHeaderView::ResizeMode mode);

  QString GetNameOfDivingSite(int diving_site_id) const;

  QItemSelectionModel *GetSelectionModel();
  auto GetDiveAtRow(int row) const {
    return m_model.GetDiveAtIndex(m_model.index(row, 0));
  }

  void SetSelectedDives(const std::set<int> &dive_ids);

 public slots:
  void RefreshFromDB(int diver_id = -1);
  void on_tableView_doubleClicked(const QModelIndex &);

 private slots:
  void on_tableView_clicked(const QModelIndex &index);

 private:
  Ui::DiveSearch *ui;

  cpsm::DivesViewModel m_model;
};

}  // namespace gui
