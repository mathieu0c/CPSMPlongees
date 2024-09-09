#include "DiverSearch.hpp"

#include "ui_DiverSearch.h"

namespace gui {

DiverSearch::DiverSearch(QWidget *parent) : DiverSearch{new cpsm::DiversViewModel{parent}, parent} {}

DiverSearch::DiverSearch(cpsm::DiversViewModel *model, QWidget *parent)
    : QWidget(parent), ui(new Ui::DiverSearch), m_model{model} {
  ui->setupUi(this);

  SetModel(m_model);

  ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableView->setFocusPolicy(Qt::NoFocus);
}

DiverSearch::~DiverSearch() {
  delete ui;
}

std::optional<cpsm::DiverWithDiveCount> DiverSearch::GetSelectedDiver() const {
  const auto kSelection{ui->tableView->selectionModel()};
  if (!kSelection->hasSelection()) {
    return {};
  }
  if (kSelection->selectedRows().size() != 1) {
    return {};
  }
  return m_model->GetDiverAtIndex(kSelection->currentIndex());
}

QVector<cpsm::DiverWithDiveCount> DiverSearch::GetSelectedDivers() const {
  const auto kSelection{ui->tableView->selectionModel()};
  if (!kSelection->hasSelection()) {
    return {};
  }

  QVector<cpsm::DiverWithDiveCount> out{};
  for (const auto &index : kSelection->selectedRows()) {
    if (const auto kDiverOpt{m_model->GetDiverAtIndex(index)}; kDiverOpt) {
      out.append(kDiverOpt.value());
    }
  }

  return out;
}

void DiverSearch::SetSectionResizeMode(QHeaderView::ResizeMode mode) {
  ui->tableView->horizontalHeader()->setSectionResizeMode(mode);
}

void DiverSearch::SetItemDelegateForColumn(int column, QAbstractItemDelegate *delegate) {
  ui->tableView->setItemDelegateForColumn(column, delegate);
  SPDLOG_DEBUG("Delegate set!");
}

void DiverSearch::SetModel(cpsm::DiversViewModel *model) {
  disconnect(m_model, nullptr, this, nullptr); /* Disconnect old model from widget */

  m_model = model;
  ui->tableView->setModel(m_model);

  connect(ui->cb_member, &QCheckBox::stateChanged, m_model, [this](int state) {
    m_model->SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterIsMember, state != Qt::CheckState::Unchecked);
    m_model->SetFilterNegate(cpsm::DiversViewModel::Filters::kFilterIsMember,
                             state == Qt::CheckState::PartiallyChecked);
    ui->lbl_member_state->setText(state == Qt::CheckState::Checked
                                      ? tr("(oui)")
                                      : (state == Qt::CheckState::PartiallyChecked ? tr("(non)") : ""));
  });

  connect(ui->cb_registered, &QCheckBox::stateChanged, m_model, [this](int state) {
    m_model->SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterIsCurrentlyRegistered,
                              state != Qt::CheckState::Unchecked);
    m_model->SetFilterNegate(cpsm::DiversViewModel::Filters::kFilterIsCurrentlyRegistered,
                             state == Qt::CheckState::PartiallyChecked);
    ui->lbl_registered_state->setText(state == Qt::CheckState::Checked
                                          ? tr("(oui)")
                                          : (state == Qt::CheckState::PartiallyChecked ? tr("(non)") : ""));
  });

  connect(ui->cb_positive_balance, &QCheckBox::stateChanged, m_model, [this](bool state) {
    if (state) {
      ui->cb_negative_balance->setChecked(false);
    }
    m_model->SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterPositiveBalance, state);
  });

  connect(ui->cb_negative_balance, &QCheckBox::stateChanged, m_model, [this](bool state) {
    if (state) {
      ui->cb_positive_balance->setChecked(false);
    }
    m_model->SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterNegativeBalance, state);
  });

  connect(ui->le_name, &QLineEdit::textChanged, m_model, &cpsm::DiversViewModel::SetNameFilter);

  auto lambda_update_result_count = [this]() {
    ui->lbl_status->setText(tr("%1 plongeurs trouvés").arg(m_model->rowCount()));
  };
  connect(m_model, &cpsm::DiversViewModel::rowsInserted, this, lambda_update_result_count);
  connect(m_model, &cpsm::DiversViewModel::rowsRemoved, this, lambda_update_result_count);

  if (db::Def().isOpen()) {
    RefreshFromDB();
  }
}

void DiverSearch::RefreshFromDB() {
  m_model->LoadFromDB();
}

void DiverSearch::SetSelectedRows(const std::set<int> &rows) {
  ui->tableView->clearSelection();
  if (rows.empty()) {
    return;
  }

  QItemSelectionModel *selectionModel = ui->tableView->selectionModel();
  if (selectionModel) {
    QItemSelection selection{};
    for (const auto &row : rows) {
      selection.select(m_model->index(row, 0), m_model->index(row, m_model->columnCount() - 1));
    }

    selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}

void DiverSearch::on_tableView_doubleClicked(const QModelIndex &) {
  const auto kSelectedDiverOpt{GetSelectedDiver()};
  if (!kSelectedDiverOpt) {
    return;
  }

  const auto &selected_diver{kSelectedDiverOpt.value()};
  emit DoubleClickOnDiver(selected_diver);
}

}  // namespace gui
