#include "DiveDetails.hpp"

#include <CPSMGlobals.hpp>

#include "ui_DiveDetails.h"

namespace gui {

DiveDetails::DiveDetails(QWidget* parent) : QWidget(parent), ui(new Ui::DiveDetails) {
  ui->setupUi(this);

  m_base_groupbox_title = ui->groupBox->title();

  ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  ui->tableView->setModel(&m_model);

  ui->groupBox->setTitle(m_base_groupbox_title.arg("", "", ""));
}

DiveDetails::~DiveDetails() {
  delete ui;
}

void DiveDetails::SetDive(const cpsm::db::Dive& dive, const QString& diving_site_name) {
  m_model.LoadFromDB(dive.dive_id);

  ui->groupBox->setTitle(m_base_groupbox_title.arg(diving_site_name,
                                                   dive.datetime.date().toString(cpsm::consts::kDateUserFormat),
                                                   dive.datetime.time().toString(cpsm::consts::kTimeFormat)));
}

void DiveDetails::Clear() {
  m_model.Clear();
  ui->groupBox->setTitle(m_base_groupbox_title.arg("", "", ""));
}

}  // namespace gui

#include "moc_DiveDetails.cpp"
