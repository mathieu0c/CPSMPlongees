#include <QKeyEvent>
#include <QLineEdit>

#include <Logger/logger.hpp>

#include "Gui/GuiUtils.hpp"

class LineEditUpperCase : public QLineEdit {
 public:
  LineEditUpperCase(QWidget *parent = nullptr) : QLineEdit(parent) {}

 protected:
  void keyPressEvent(QKeyEvent *event) override {
    QString text = event->text();
    if (!text.isEmpty() && text[0].isLetter()) {
      text = text.toUpper();
      insert(text);
    } else {
      QLineEdit::keyPressEvent(event);
    }
  }
};

/* Uppercase on word start */
class LineEditAutoUpperCase : public QLineEdit {
 public:
  LineEditAutoUpperCase(QWidget *parent = nullptr) : QLineEdit(parent) {}

 protected:
  void keyPressEvent(QKeyEvent *event) override {
    QLineEdit::keyPressEvent(event);

    setText(WordStartUppercase(this->text()));
  }
};
