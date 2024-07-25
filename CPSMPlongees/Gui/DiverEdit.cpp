#include "DiverEdit.hpp"

#include <QMessageBox>

#include <Models/DivesViewModelForDiverEdit.hpp>

#include "Constants.hpp"
#include "ui_DiverEdit.h"

namespace gui {

DiverEdit::DiverEdit(QWidget *parent) : QWidget(parent), ui(new Ui::DiverEdit) {
  ui->setupUi(this);

  ui->diveSearch->SetModel(new cpsm::DivesViewModelForDiverEdit{this});

  auto lambda_connect_editing_finished_sub{
      [this]<typename Editor, typename EditorBis, typename MemberType, typename ReturnType>(
          Editor *edit, ReturnType (EditorBis::*editor_data_func)() const, auto MemberType::*target_member) {
        connect(edit, &Editor::editingFinished, this, [this, target_member, edit, editor_data_func]() {
          const auto &data{(edit->*editor_data_func)()};
          if constexpr (std::is_base_of_v<cpsm::db::Diver, MemberType>) {
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

  auto lambda_connect_checkbox{[this](QCheckBox *cb, int cpsm::db::Diver::*target_member) {
    connect(cb, &QCheckBox::toggled, this, [this, target_member](bool checked) {
      m_diver.*target_member = checked;
      if (!m_inhibit_all_gear_checkbox_change) {
        ui->cb_gear_global->setChecked(AllGearChecked());
      }
    });
  }};

  lambda_connect_editing_finished(ui->le_lastname, &cpsm::db::Diver::last_name);
  lambda_connect_editing_finished(ui->le_firstname, &cpsm::db::Diver::first_name);
  lambda_connect_editing_finished(ui->de_birthDate, &cpsm::db::Diver::birth_date);
  connect(ui->de_birthDate, &QDateEdit::dateChanged, this, [this]() {
    ui->lbl_age->setText(QString::number(cpsm::db::GetDiverAge(ui->de_birthDate->date())));
  });
  lambda_connect_editing_finished(ui->le_license, &cpsm::db::Diver::license_number);
  lambda_connect_editing_finished(ui->de_registration, &cpsm::db::Diver::registration_date);
  lambda_connect_editing_finished(ui->de_certificate, &cpsm::db::Diver::certif_date);
  lambda_connect_editing_finished(ui->le_address, &cpsm::db::DiverAddress::address);
  lambda_connect_editing_finished(ui->le_city, &cpsm::db::DiverAddress::city);
  lambda_connect_editing_finished(ui->le_postalCode, &cpsm::db::DiverAddress::postal_code);
  lambda_connect_editing_finished(ui->le_mail, &cpsm::db::Diver::email);
  lambda_connect_editing_finished(ui->le_phone, &cpsm::db::Diver::phone_number);

  /* -- Manual connections -- */

  /* Gear */
  lambda_connect_checkbox(ui->cb_computer, &cpsm::db::Diver::gear_computer);
  lambda_connect_checkbox(ui->cb_jacket, &cpsm::db::Diver::gear_jacket);
  lambda_connect_checkbox(ui->cb_regulator, &cpsm::db::Diver::gear_regulator);
  lambda_connect_checkbox(ui->cb_suit, &cpsm::db::Diver::gear_suit);
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
  connect(ui->cb_member, &QCheckBox::toggled, this, [this](bool checked) {
    if (checked) {
      ui->de_member->setDate(QDate::currentDate());
    } else {
      if (m_inhibit_member_checkbox_change) {
        return;
      }
      /* Member unchecked -> restore original diver member date if he was not a member. Set Epoch otherwise */
      m_inhibit_member_checkbox_change = true;
      ui->de_member->setDate(cpsm::db::IsDiverCurrentlyAMember(m_original_diver) ? cpsm::consts::kEpochDate
                                                                                 : m_original_diver.member_date);
      m_inhibit_member_checkbox_change = false;
    }
  });
  connect(ui->de_member, &QDateEdit::dateChanged, this, [this](const QDate &date) {
    if (date.year() != m_diver.member_date.year()) { /* Update date only if the year is different */
      m_diver.member_date = date;
    }
    if (m_inhibit_member_checkbox_change) {
      return;
    }
    m_inhibit_member_checkbox_change = true;
    ui->cb_member->setChecked(m_diver.member_date.year() == QDate::currentDate().year());
    m_inhibit_member_checkbox_change = false;
  });

  /* Address */
  connect(ui->pb_quit_family, &QPushButton::clicked, this, [this]() {
    m_address.address_id = {};
    SetAddress(m_address);
  });
}

DiverEdit::~DiverEdit() {
  delete ui;
}

void DiverEdit::RefreshFromDB() {
  /* Fill diver level cb */
  const auto kLevelList{db::readLFromDB<cpsm::db::DiverLevel>(
      db::Def(), cpsm::db::ExtractDiverLevel, "SELECT * FROM %0", {cpsm::db::DiverLevel::db_table}, {})};
  for (const auto &e : kLevelList) {
    ui->cb_level->addItem(e.level_name, e.diver_level_id);
  }

  ui->diveSearch->RefreshFromDB(m_diver.diver_id);
}

bool DiverEdit::SetDiver(const cpsm::db::Diver &diver, int dive_count) {
  m_dive_count = dive_count;
  m_diver = diver;
  m_original_diver = diver;
  m_diver_original_paid_dives_count = m_diver.paid_dives;

  ui->sb_payment->setValue(0);

  const auto kSuccess{SetDiverAddressFromId(m_diver.address_id)};
  UpdateUiFromDiver();
  return kSuccess;
}

void DiverEdit::SetAddress(const cpsm::db::DiverAddress &address) {
  m_address = address;
  m_original_address = address;
  m_diver.address_id = address.address_id; /* Shouldn't be required... But meh */
  UpdateAddressUi();

  auto database{db::Def()};
  const auto kDiverWithAddressCount{
      db::queryCount(database,
                     "SELECT %0 FROM %1 WHERE %2 = ?",
                     {cpsm::db::Diver::diver_id_col, cpsm::db::Diver::db_table, cpsm::db::Diver::address_id_col},
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

bool DiverEdit::WasEdited() const {
  return m_diver != m_original_diver || m_address != m_original_address;
}

bool DiverEdit::SetDiverAddressFromId(int address_id) {
  std::optional<cpsm::db::DiverAddress> addr{};

  if (address_id > 0) {
    addr = cpsm::db::GetDiverAddressFromId(db::Def(), {address_id});
    if (!addr.has_value()) {
      SPDLOG_ERROR("Failed to retrieve address with id: <{}>", address_id);
      return false;
    }
  }

  SetAddress(addr.has_value() ? addr.value() : cpsm::db::DiverAddress{}); /* If not found or new, set empty address */

  UpdateAddressUi();
  return true;
}

void DiverEdit::UpdateUiFromDiver() {
  ui->le_firstname->setText(m_diver.first_name);
  ui->le_lastname->setText(m_diver.last_name.toUpper());
  ui->de_birthDate->setDate(m_diver.birth_date);
  ui->lbl_age->setText(QString::number(cpsm::db::GetDiverAge(ui->de_birthDate->date())));
  ui->le_license->setText(m_diver.license_number);
  ui->de_registration->setDate(m_diver.registration_date);
  ui->cb_member->setChecked(cpsm::db::IsDiverCurrentlyAMember(m_diver));
  ui->de_member->setDate(m_diver.member_date.isValid() ? m_diver.member_date : cpsm::consts::kEpochDate);
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

  ui->diveSearch->RefreshFromDB(m_diver.diver_id);
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
                       {cpsm::db::Diver::diver_id_col, cpsm::db::Diver::db_table, cpsm::db::Diver::address_id_col},
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
      auto ans{
          QMessageBox::warning(this,
                               tr("Attention"),
                               tr("La modification de l'adresse va impacter %0 autre(s) plongeur(s).\nÊtes-vous sûr ?")
                                   .arg(kDiverWithAddressCount - 1),
                               QMessageBox::Ok | QMessageBox::Cancel)};
      if (ans == QMessageBox::Cancel) {
        return;
      }
    }
  }

  emit DiverEdited({{m_diver, m_address}});
  m_original_address = m_address;
}

void DiverEdit::OnCancelled() {
  if (!WasEdited()) {
    emit DiverEdited({});
    return;
  }
  auto ans{QMessageBox::question(
      this,
      tr("Confirmation"),
      tr("Toute modification sera définitivement perdue.\nSouhaitez-vous abandonner toutes les modifications ?"),
      QMessageBox::Yes | QMessageBox::No)};
  if (ans == QMessageBox::Yes) {
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
