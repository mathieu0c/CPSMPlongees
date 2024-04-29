#include "DiverEdit.hpp"

#include "ui_DiverEdit.h"

namespace gui {

DiverEdit::DiverEdit(QWidget *parent) : QWidget(parent), ui(new Ui::DiverEdit) {
  ui->setupUi(this);
}

DiverEdit::~DiverEdit() {
  delete ui;
}

bool DiverEdit::SetDiver(const db::Diver &diver) {
  //
  m_diver = diver;

  const auto kAddr = db::GetDiverAddressFromId(db::Def(), {m_diver.address_id});

  if (!kAddr.has_value()) {
    return false;
  }
  m_address = kAddr.value();

  UpdateUi();
  return true;
}

void DiverEdit::UpdateUi() {
  ui->le_firstname->setText(m_diver.first_name);
  ui->le_lastname->setText(m_diver.last_name);
  ui->de_birthDate->setDate(QDate::fromString(m_diver.birth_date));
  ui->le_license->setText(m_diver.license_number);
  ui->cb_member->setChecked(m_diver.is_member);
  // Combobox level to do
  ui->de_certificate->setDate(QDate::fromString(m_diver.certif_date));

  /* -- Contact -- */
  ui->le_address->setText(m_address.address);
  ui->le_postalCode->setText(m_address.postal_code);
  ui->le_city->setText(m_address.city);
  ui->le_mail->setText(m_diver.email);
  ui->le_phone->setText(m_diver.phone_number);
}

}  // namespace gui
