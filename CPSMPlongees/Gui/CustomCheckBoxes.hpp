#pragma once

#include <QCheckBox>
#include <QObject>

class ReversedTristateCheckBox : public QCheckBox {
  Q_OBJECT
 public:
  ReversedTristateCheckBox(QWidget *parent = nullptr);
  ReversedTristateCheckBox(const QString &text, QWidget *parent = nullptr);

 protected:
  void nextCheckState() override;
};

/* A checkbox that the user can only check and not uncheck */
class OnlyCheckableCheckBox : public QCheckBox {
  Q_OBJECT
 public:
  OnlyCheckableCheckBox(QWidget *parent = nullptr);
  OnlyCheckableCheckBox(const QString &text, QWidget *parent = nullptr);

 protected:
  void nextCheckState() override;
};
