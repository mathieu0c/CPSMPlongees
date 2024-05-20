#include "GuiUtils.hpp"

StatusLabel::StatusLabel(QWidget *parent, Qt::WindowFlags f) : QLabel{parent, f} {
  this->hide();
}

StatusLabel::StatusLabel(const QString &text, QWidget *parent, Qt::WindowFlags f) : QLabel{text, parent, f} {
  if (text.isEmpty()) {
    this->hide();
  }
}

void StatusLabel::setText(const QString &text) {
  QLabel::setText(text);
  if (text.isEmpty()) {
    this->hide();
  } else {
    this->show();
  }
}
