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

QString WordStartUppercase(const QString &input) {
  constexpr auto kRegex{R"(\b[a-z]{1})"};
  QRegularExpression re(kRegex);
  QString output = input;

  auto match_iter = re.globalMatch(output);
  while (match_iter.hasNext()) {
    auto match = match_iter.next();
    if (match.hasMatch()) {
      output[match.capturedStart()] = output[match.capturedStart()].toUpper();
    }
  }
  return output;
}
