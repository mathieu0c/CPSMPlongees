#pragma once

#include <QWidget>
#include <RawStructs.hpp>

#include <Models/DiveDetailsViewModel.hpp>

namespace gui {

namespace Ui {
class DiveDetails;
}

class DiveDetails : public QWidget {
  Q_OBJECT

 public:
  explicit DiveDetails(QWidget *parent = nullptr);
  ~DiveDetails();

  void SetDive(const cpsm::db::Dive &dive, const QString &diving_site_name);

  void Clear();

 private:
  Ui::DiveDetails *ui;

  QString m_base_groupbox_title{};

  cpsm::DiveDetailsViewModel m_model;
};

}  // namespace gui
