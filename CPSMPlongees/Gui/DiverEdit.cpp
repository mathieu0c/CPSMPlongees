#include "DiverEdit.hpp"

#include <QMessageBox>

#include "ui_DiverEdit.h"

namespace gui {

DiverEdit::DiverEdit(QWidget *parent) : QWidget(parent), ui(new Ui::DiverEdit) {
  ui->setupUi(this);

  auto lambda_connect_editing_finished_sub{
      [this]<typename Editor, typename EditorBis, typename MemberType, typename ReturnType>(
          Editor *edit, ReturnType (EditorBis::*editor_data_func)() const, auto MemberType::*target_member) {
        connect(edit, &Editor::editingFinished, this, [this, target_member, edit, editor_data_func]() {
          const auto &data{(edit->*editor_data_func)()};
          if constexpr (std::is_base_of_v<db::Diver, MemberType>) {
            m_diver.*target_member = data;
          } else {
            m_address.*target_member = data;
          }
        });
      }};

  auto lambda_connect_editing_finished{[lambda_connect_editing_finished_sub]<typename Editor, typename MemberType>(
                                           Editor *edit, auto MemberType::*target_member) {
    if constexpr (std::is_base_of_v<QLineEdit, Editor>) {
      lambda_connect_editing_finished_sub(edit, &Editor::text, target_member);
    }

    else if constexpr (std::is_base_of_v<QDateEdit, Editor>) {
      lambda_connect_editing_finished_sub(edit, &QDateEdit::date, target_member);
    }
  }};

  auto lambda_connect_checkbox{[this](QCheckBox *cb, int db::Diver::*target_member) {
    connect(cb, &QCheckBox::toggled, this, [this, target_member](bool checked) {
      m_diver.*target_member = checked;
      if (!m_inhibit_all_gear_checkbox_change) {
        ui->cb_gear_global->setChecked(AllGearChecked());
      }
    });
  }};

  lambda_connect_editing_finished(ui->le_lastname, &db::Diver::last_name);
  lambda_connect_editing_finished(ui->le_firstname, &db::Diver::first_name);
  lambda_connect_editing_finished(ui->de_birthDate, &db::Diver::birth_date);
  lambda_connect_editing_finished(ui->le_license, &db::Diver::license_number);
  lambda_connect_editing_finished(ui->de_registration, &db::Diver::registration_date);
  lambda_connect_editing_finished(ui->de_member, &db::Diver::member_date);
  lambda_connect_editing_finished(ui->de_certificate, &db::Diver::certif_date);
  lambda_connect_editing_finished(ui->le_address, &db::DiverAddress::address);
  lambda_connect_editing_finished(ui->le_city, &db::DiverAddress::city);
  lambda_connect_editing_finished(ui->le_postalCode, &db::DiverAddress::postal_code);
  lambda_connect_editing_finished(ui->le_mail, &db::Diver::email);
  lambda_connect_editing_finished(ui->le_phone, &db::Diver::phone_number);

  /* -- Manual connections -- */

  /* Gear */
  lambda_connect_checkbox(ui->cb_computer, &db::Diver::gear_computer);
  lambda_connect_checkbox(ui->cb_jacket, &db::Diver::gear_jacket);
  lambda_connect_checkbox(ui->cb_regulator, &db::Diver::gear_regulator);
  lambda_connect_checkbox(ui->cb_suit, &db::Diver::gear_suit);
  connect(ui->cb_gear_global, &QCheckBox::clicked, this, &DiverEdit::SetAllGearChecked);

  /* Payments */
  connect(ui->sb_payment, &QSpinBox::valueChanged, this, &DiverEdit::OnPaymentValueChanged);
  connect(ui->pb_paymentPlus, &QPushButton::clicked, this, [this]() {
    ui->sb_payment->setValue(ui->sb_payment->value() + 1);
  });
  connect(ui->pb_paymentMinus, &QPushButton::clicked, this, [this]() {
    ui->sb_payment->setValue(ui->sb_payment->value() - 1);
  });

  /* Level */
  connect(ui->cb_level, &QComboBox::activated, this, [this](int index) {
    m_diver.diver_level_id = ui->cb_level->itemData(index).toInt();
  });

  /* Member */
  connect(ui->cb_member, &QCheckBox::toggled, this, [this](bool checked) { ui->de_member->setEnabled(checked); });
  connect(ui->cb_member, &QCheckBox::toggled, this, [this](bool checked) { m_diver.is_member = checked; });

  /* Address */
  connect(ui->pb_quit_family, &QPushButton::clicked, this, [this]() {
    m_address.address_id = {};
    SetAddress(m_address);
  });
}

DiverEdit::~DiverEdit() {
  delete ui;
}

void DiverEdit::OnDatabaseLoaded() {
  /* Fill diver level cb */
  const auto kLevelList{db::readLFromDB<db::DiverLevel>(
      db::Def(), db::ExtractDiverLevel, "SELECT * FROM %0", {db::DiverLevel::db_table}, {})};
  for (const auto &e : kLevelList) {
    ui->cb_level->addItem(e.level_name, e.diver_level_id);
  }
}

bool DiverEdit::SetDiver(const db::Diver &diver, int dive_count) {
  m_dive_count = dive_count;
  m_diver = diver;
  m_diver_original_paid_dives_count = m_diver.paid_dives;

  const auto kSuccess{SetDiverAddressFromId(m_diver.address_id)};
  UpdateUiFromDiver();
  return kSuccess;
}

void DiverEdit::SetAddress(const db::DiverAddress &address) {
  m_address = address;
  m_diver.address_id = address.address_id; /* Shouldn't be required... But meh */
  UpdateAddressUi();

  auto database{db::Def()};
  const auto kDiverWithAddressCount{
      db::queryCount(database,
                     "SELECT %0 FROM %1 WHERE %2 = ?",
                     {db::Diver::diver_id_col, db::Diver::db_table, db::Diver::address_id_col},
                     {m_diver.address_id})};
  if (kDiverWithAddressCount > 1) {
    ui->lbl_disp_diver_with_address_count->setVisible(true);
    ui->lbl_diver_with_address_count->setVisible(true);
    ui->pb_quit_family->setVisible(true);
  } else {
    ui->lbl_disp_diver_with_address_count->setVisible(false);
    ui->lbl_diver_with_address_count->setVisible(false);
    ui->pb_quit_family->setVisible(false);
  }

  ui->lbl_diver_with_address_count->setText(QString::number(kDiverWithAddressCount - 1));
}

bool DiverEdit::SetDiverAddressFromId(int address_id) {
  if (address_id <= 0) {
    m_diver.address_id = 0;
    UpdateAddressUi();
  }

  const auto kAddr = db::GetDiverAddressFromId(db::Def(), {m_diver.address_id});
  m_original_address = kAddr ? kAddr.value() : db::DiverAddress{};

  if (!kAddr.has_value()) {
    UpdateAddressUi();
    return false;
  }
  SetAddress(kAddr.value());
  return true;
}

void DiverEdit::UpdateUiFromDiver() {
  ui->le_firstname->setText(m_diver.first_name);
  ui->le_lastname->setText(m_diver.last_name.toUpper());
  ui->de_birthDate->setDate(m_diver.birth_date);
  ui->le_license->setText(m_diver.license_number);
  ui->de_registration->setDate(m_diver.registration_date);
  ui->cb_member->setChecked(m_diver.is_member);
  ui->de_member->setEnabled(m_diver.is_member);
  ui->de_member->setDate(m_diver.member_date);
  SetLevelComboboxFromLevelId(m_diver.diver_level_id);
  ui->de_certificate->setDate(m_diver.certif_date);

  /* -- Contact -- */
  UpdateAddressUi();
  ui->le_mail->setText(m_diver.email);
  ui->le_phone->setText(m_diver.phone_number);

  /* -- Dive balance -- */
  ui->sb_diveCount->setValue(m_dive_count);
  UpdateUiSold();
  ui->cb_regulator->setChecked(m_diver.gear_regulator);
  ui->cb_suit->setChecked(m_diver.gear_suit);
  ui->cb_computer->setChecked(m_diver.gear_computer);
  ui->cb_jacket->setChecked(m_diver.gear_jacket);
}

void DiverEdit::UpdateAddressUi() {
  ui->le_address->setText(m_address.address);
  ui->le_postalCode->setText(m_address.postal_code);
  ui->le_city->setText(m_address.city);
}

void DiverEdit::SetLevelComboboxFromLevelId(int level_id) {
  //
  const auto kFindResults{ui->cb_level->findData(level_id)};
  if (kFindResults != -1) {
    ui->cb_level->setCurrentIndex(kFindResults);
  } else {
    SPDLOG_WARN("Failed to find level id: <{}> in diver edit combobox", level_id);
  }
}

bool DiverEdit::AllGearChecked() const {
  return ui->cb_computer->isChecked() && ui->cb_jacket->isChecked() && ui->cb_regulator->isChecked() &&
         ui->cb_suit->isChecked();
}

void DiverEdit::SetAllGearChecked(bool checked) {
  SPDLOG_DEBUG("All gear checked?: {}", AllGearChecked());

  m_inhibit_all_gear_checkbox_change = true;
  ui->cb_computer->setChecked(checked);
  ui->cb_jacket->setChecked(checked);
  ui->cb_regulator->setChecked(checked);
  ui->cb_suit->setChecked(checked);
  m_inhibit_all_gear_checkbox_change = false;
}

void DiverEdit::OnPaymentValueChanged(int new_val) {
  SPDLOG_DEBUG("New val: {}", new_val);
  m_diver.paid_dives = new_val + m_diver_original_paid_dives_count;
  UpdateUiSold();
}

void DiverEdit::UpdateUiSold() {
  ui->sb_sold->setValue(m_diver.paid_dives - m_dive_count);
}

void DiverEdit::OnOk() {
  /* Only if there was modifications on the address */
  if (m_original_address != m_address) {
    auto database{db::Def()};
    const auto kDiverWithAddressCount{
        db::queryCount(database,
                       "SELECT %0 FROM %1 WHERE %2 = ?",
                       {db::Diver::diver_id_col, db::Diver::db_table, db::Diver::address_id_col},
                       {m_diver.address_id})};
    if (kDiverWithAddressCount < 0) {
      SPDLOG_ERROR("Failed to count diver with address id: <{}>", m_diver.address_id);
      QMessageBox::critical(this,
                            tr("Erreur"),
                            tr("Impossible de vérifier le nombre de plongeurs impactés par l'adresse "
                               "modifiée.\nAnnulation de la sauvegarde des modifications par sécurité."));
      return;
    }

    if (kDiverWithAddressCount > 1) {
      auto ans{QMessageBox::warning(
          this,
          tr("Attention"),
          tr("La modification de l'adresse va impacter %0 plongeurs.\nÊtes-vous sûr ?").arg(kDiverWithAddressCount),
          QMessageBox::Ok | QMessageBox::Cancel)};
      if (ans == QMessageBox::Cancel) {
        return;
      }
    }
  }

  SPDLOG_DEBUG("Diver edited: {}", m_diver);
  SPDLOG_DEBUG("Address edited: {}", m_address);
  emit DiverEdited({{m_diver, m_address}});
}

void DiverEdit::OnCancelled() {
  auto ans{QMessageBox::question(this,
                                 tr("Confirmation"),
                                 tr("Toute modification sera définitivement perdue.\nAnnuler ?"),
                                 QMessageBox::Yes | QMessageBox::No)};
  if (ans == QMessageBox::Yes) {
    SPDLOG_DEBUG("Cancelled diver edition");
    emit DiverEdited({});
  }
}

void DiverEdit::on_buttonBox_accepted() {
  OnOk();
}

void DiverEdit::on_buttonBox_rejected() {
  OnCancelled();
}

}  // namespace gui
