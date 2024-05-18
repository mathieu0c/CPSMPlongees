#include "DiverSearch.hpp"

#include "ui_DiverSearch.h"

namespace gui {

DiverSearch::DiverSearch(QWidget *parent) : QWidget(parent), ui(new Ui::DiverSearch) {
  ui->setupUi(this);

  ui->tableView->setModel(&m_model);

  connect(ui->cb_member, &QCheckBox::stateChanged, &m_model, [this](int state) {
    m_model.SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterIsMember, state != Qt::CheckState::Unchecked);
    m_model.SetFilterNegate(cpsm::DiversViewModel::Filters::kFilterIsMember, state == Qt::CheckState::PartiallyChecked);
  });

  connect(ui->cb_registered, &QCheckBox::stateChanged, &m_model, [this](int state) {
    m_model.SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterIsCurrentlyRegistered,
                             state != Qt::CheckState::Unchecked);
    m_model.SetFilterNegate(cpsm::DiversViewModel::Filters::kFilterIsCurrentlyRegistered,
                            state == Qt::CheckState::PartiallyChecked);
  });

  connect(ui->cb_positive_balance, &QCheckBox::stateChanged, &m_model, [this](bool state) {
    if (state) {
      ui->cb_negative_balance->setChecked(false);
    }
    m_model.SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterPositiveBalance, state);
  });

  connect(ui->cb_negative_balance, &QCheckBox::stateChanged, &m_model, [this](bool state) {
    if (state) {
      ui->cb_positive_balance->setChecked(false);
    }
    m_model.SetFilterEnabled(cpsm::DiversViewModel::Filters::kFilterNegativeBalance, state);
  });

  connect(ui->le_name, &QLineEdit::textChanged, &m_model, &cpsm::DiversViewModel::SetNameFilter);
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
  return m_model.GetDiverAtIndex(kSelection->currentIndex());
}

QVector<cpsm::DiverWithDiveCount> DiverSearch::GetSelectedDivers() const {
  const auto kSelection{ui->tableView->selectionModel()};
  if (!kSelection->hasSelection()) {
    return {};
  }

  QVector<cpsm::DiverWithDiveCount> out{};
  for (const auto &index : kSelection->selectedRows()) {
    if (const auto kDiverOpt{m_model.GetDiverAtIndex(index)}; kDiverOpt) {
      out.append(kDiverOpt.value());
    }
  }

  return out;
}

void DiverSearch::RefreshFromDB() {
  m_model.LoadFromDB();
}

void DiverSearch::on_tableView_doubleClicked(const QModelIndex &) {
  const auto kSelectedDiverOpt{GetSelectedDiver()};
  if (!kSelectedDiverOpt) {
    return;
  }

  const auto &selected_diver{kSelectedDiverOpt.value()};
  DoubleClickOnDiver(selected_diver);
}

}  // namespace gui
