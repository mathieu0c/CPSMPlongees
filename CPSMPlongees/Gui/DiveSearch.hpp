#pragma once

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

 public slots:
  void RefreshFromDB();
  void on_tableView_doubleClicked(const QModelIndex &);

 private:
  Ui::DiveSearch *ui;

  cpsm::DivesViewModel m_model;
};

}  // namespace gui
