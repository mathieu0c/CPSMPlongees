#include "CustomCheckBoxes.hpp"

#include <Logger/logger.hpp>

ReversedTristateCheckBox::ReversedTristateCheckBox(QWidget *parent) : QCheckBox(parent) {}

ReversedTristateCheckBox::ReversedTristateCheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent) {}

void ReversedTristateCheckBox::nextCheckState() {
  if (!isCheckable()) {
    return;
  }

  if (!isTristate()) {
    setChecked(!isChecked());
    return;
  }

  switch (checkState()) {
    case Qt::CheckState::Unchecked:
      setCheckState(Qt::CheckState::Checked);
      break;
    case Qt::CheckState::PartiallyChecked:
      setCheckState(Qt::CheckState::Unchecked);
      break;
    case Qt::CheckState::Checked:
      setCheckState(Qt::CheckState::PartiallyChecked);
      break;
    default:
      SPDLOG_ERROR("Invalid check state: {}", checkState());
      break;
  }
}

OnlyCheckableCheckBox::OnlyCheckableCheckBox(QWidget *parent) : QCheckBox(parent) {}

OnlyCheckableCheckBox::OnlyCheckableCheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent) {}

void OnlyCheckableCheckBox::nextCheckState() {
  if (!isCheckable()) {
    return;
  }

  if (!isTristate()) {
    setChecked(true);
    return;
  }
}
