#include "DiveEdit.hpp"

#include <QMessageBox>

#include "ui_DiveEdit.h"

namespace gui {

DiveEdit::DiveEdit(QWidget *parent) : QWidget(parent), ui(new Ui::DiveEdit) {
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DiveEdit::OnOk);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DiveEdit::OnCancelled);
}

DiveEdit::~DiveEdit() {
  delete ui;
}

bool DiveEdit::SetDive(const cpsm::db::Dive &dive) {
  bool success{true};

  m_dive.dive = dive;
  m_original_dive.dive = dive;

  UpdateUiFromDive();
  return success;
}

void DiveEdit::RefreshFromDB() {
  auto level_list{db::readLFromDB<cpsm::db::DivingSite>(
      db::Def(), cpsm::db::ExtractDivingSite, "SELECT * FROM %0", {cpsm::db::DivingSite::db_table}, {})};
  std::sort(level_list.begin(), level_list.end(), [](const auto &lhs, const auto &rhs) {
    return lhs.site_name < rhs.site_name;
  });
  if (!level_list.empty()) {
    ui->cb_diveSite->clear();
  }
  for (const auto &e : level_list) {
    ui->cb_diveSite->addItem(e.site_name, e.diving_site_id);
  }
}

bool DiveEdit::WasEdited() const {
  return m_dive != m_original_dive;
}

void DiveEdit::UpdateUiFromDive() {
  ui->de_diveDate->setDate(m_dive.dive.datetime.date());
  ui->te_diveTime->setTime(m_dive.dive.datetime.time());

  SetDivingSiteComboboxFromSiteId(m_dive.dive.diving_site_id);
}

void DiveEdit::SetDivingSiteComboboxFromSiteId(int site_id) {
  const auto kFindResults{ui->cb_diveSite->findData(site_id)};
  if (kFindResults != -1) {
    ui->cb_diveSite->setCurrentIndex(kFindResults);
  } else {
    SPDLOG_WARN("Failed to find site id: <{}> in diver edit combobox", site_id);
  }
}

void DiveEdit::OnOk() {
  emit DiveEdited({m_dive});
  m_original_dive = m_dive;
}

void DiveEdit::OnCancelled() {
  if (!WasEdited()) {
    emit DiveEdited({});
    return;
  }

  auto ans{QMessageBox::question(
      this,
      tr("Confirmation"),
      tr("Toute modification sera définitivement perdue.\nSouhaitez-vous abandonner toutes les modifications ?"),
      QMessageBox::Yes | QMessageBox::No)};
  if (ans == QMessageBox::Yes) {
    emit DiveEdited({});
  }
}

}  // namespace gui
