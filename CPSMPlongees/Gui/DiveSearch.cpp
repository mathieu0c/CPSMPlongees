#include "DiveSearch.hpp"

#include "ui_DiveSearch.h"

namespace gui {

DiveSearch::DiveSearch(QWidget *parent) : QWidget(parent), ui(new Ui::DiveSearch) {
  ui->setupUi(this);

  ui->tableView->setModel(&m_model);

  ui->de_end->setDate(QDate::currentDate());

  /* Morning & afternoon filters */
  auto lambda_refresh_filters{[this]() {
    m_model.SetFilterEnabled(cpsm::DivesViewModel::kFilterMorning,
                             ui->cb_morning->isChecked() && !ui->cb_afternoon->isChecked());
    m_model.SetFilterEnabled(cpsm::DivesViewModel::kFilterAfternoon,
                             !ui->cb_morning->isChecked() && ui->cb_afternoon->isChecked());
  }};
  connect(ui->cb_morning, &QCheckBox::stateChanged, this, lambda_refresh_filters);
  connect(ui->cb_afternoon, &QCheckBox::stateChanged, this, lambda_refresh_filters);
}

DiveSearch::~DiveSearch() {
  delete ui;
}

std::optional<cpsm::DisplayDive> DiveSearch::GetSelectedDive() const {
  const auto kSelection{ui->tableView->selectionModel()};
  if (!kSelection->hasSelection()) {
    return {};
  }
  if (kSelection->selectedRows().size() != 1) {
    return {};
  }
  return m_model.GetDiveAtIndex(kSelection->currentIndex());
}

QVector<cpsm::DisplayDive> DiveSearch::GetSelectedDives() const {
  const auto kSelection{ui->tableView->selectionModel()};
  if (!kSelection->hasSelection()) {
    return {};
  }

  QVector<cpsm::DisplayDive> out{};
  for (const auto &index : kSelection->selectedRows()) {
    if (const auto kDiverOpt{m_model.GetDiveAtIndex(index)}; kDiverOpt) {
      out.append(kDiverOpt.value());
    }
  }

  return out;
}

void DiveSearch::RefreshFromDB() {
  m_model.LoadFromDB();
}

void DiveSearch::on_tableView_doubleClicked(const QModelIndex &) {
  const auto kSelectedDiverOpt{GetSelectedDive()};
  if (!kSelectedDiverOpt) {
    return;
  }

  const auto &selected_diver{kSelectedDiverOpt.value()};
  std::ignore = selected_diver;
  // DoubleClickOnDiver(selected_diver);
}

}  // namespace gui
