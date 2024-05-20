#pragma once

#include <QLabel>
#include <QObject>

/* Label that goes invisible when empty text */
class StatusLabel : public QLabel {
  Q_OBJECT
 public:
  StatusLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  StatusLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

  void setText(const QString &text);
};
