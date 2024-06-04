#include "DiveSearch.hpp"

#include <RawStructs.hpp>

#include "ui_DiveSearch.h"

namespace gui {

DiveSearch::DiveSearch(QWidget *parent) : QWidget(parent), ui(new Ui::DiveSearch) {
  ui->setupUi(this);

  ui->tableView->setModel(&m_model);

  ui->de_end->setDate(QDate::currentDate());

  /* Morning & afternoon filters */
  auto lambda_refresh_morning_afternoon_filters{[this]() {
    m_model.SetFilterEnabled(cpsm::DivesViewModel::kFilterMorning,
                             ui->cb_morning->isChecked() && !ui->cb_afternoon->isChecked());
    m_model.SetFilterEnabled(cpsm::DivesViewModel::kFilterAfternoon,
                             !ui->cb_morning->isChecked() && ui->cb_afternoon->isChecked());
  }};
  connect(ui->cb_morning, &QCheckBox::stateChanged, this, lambda_refresh_morning_afternoon_filters);
  connect(ui->cb_afternoon, &QCheckBox::stateChanged, this, lambda_refresh_morning_afternoon_filters);

  auto lambda_refresh_date_filter{[this]() { m_model.SetDateFilter(ui->de_start->date(), ui->de_end->date()); }};
  connect(ui->de_start, &QDateEdit::dateChanged, this, lambda_refresh_date_filter);
  connect(ui->de_end, &QDateEdit::dateChanged, this, lambda_refresh_date_filter);

  auto lambda_update_result_count = [this]() {
    ui->lbl_status->setText(tr("%1 plongées trouvées").arg(m_model.rowCount()));
  };
  connect(&m_model, &cpsm::DivesViewModel::rowsInserted, this, lambda_update_result_count);
  connect(&m_model, &cpsm::DivesViewModel::rowsRemoved, this, lambda_update_result_count);

  auto lambda_refresh_type_filter{
      [this](auto) { m_model.SetTypeFilter(ui->combobox_type->currentText(), ui->cb_type->isChecked()); }};
  connect(ui->combobox_type, &QComboBox::currentIndexChanged, this, lambda_refresh_type_filter);
  connect(ui->cb_type, &QCheckBox::stateChanged, this, lambda_refresh_type_filter);

  auto lambda_refresh_diver_count_filter{[this](auto) {
    std::function<bool(int, int)> operator_func;
    /* Depends on ui->cb_diver_count_operator */
    switch (ui->cb_diver_count_operator->currentIndex()) {
      case 0:
        operator_func = std::greater<int>{};
        break;
      case 1:
        operator_func = std::equal_to<int>{};
        break;
      case 2:
        operator_func = std::less<int>{};
        break;
      default:
        SPDLOG_WARN("Failed to find corresponding comparison operator for <ui->cb_diver_count_operator> index: {}",
                    ui->cb_diver_count_operator->currentIndex());
        break;
    }
    m_model.SetDiverCountFilter(operator_func, ui->sb_diver_count->value(), ui->cb_diver_count->isChecked());
  }};
  connect(ui->cb_diver_count, &QCheckBox::stateChanged, this, lambda_refresh_diver_count_filter);
  connect(ui->sb_diver_count, &QSpinBox::valueChanged, this, lambda_refresh_diver_count_filter);
  connect(ui->cb_diver_count_operator, &QComboBox::currentIndexChanged, this, lambda_refresh_diver_count_filter);
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

  const auto kTypeList{::db::readLFromDB<cpsm::db::DivingType>(
      ::db::Def(), &cpsm::db::ExtractDivingType, "SELECT * FROM %0", {cpsm::db::DivingType::db_table}, {})};

  ui->combobox_type->clear();
  for (const auto &type : kTypeList) {
    ui->combobox_type->addItem(type.type_name, type.diving_type_id);
  }
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
