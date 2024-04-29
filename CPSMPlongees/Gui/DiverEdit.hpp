#pragma once

#include <QWidget>
#include <RawStructs.hpp>

namespace gui {

namespace Ui {
class DiverEdit;
}

class DiverEdit : public QWidget {
  Q_OBJECT

 signals:
  void DiverEdited();

 public:
  explicit DiverEdit(QWidget *parent = nullptr);
  ~DiverEdit();

  bool SetDiver(const db::Diver &diver);

 private:
  void UpdateUi();

 private:
  Ui::DiverEdit *ui;

  db::Diver m_diver{};
  db::DiverAddress m_address{};
};

}  // namespace gui
