#pragma once

#include <QWidget>

namespace gui {

namespace Ui {
class DiverEdit;
}

class DiverEdit : public QWidget
{
    Q_OBJECT

public:
    explicit DiverEdit(QWidget *parent = nullptr);
    ~DiverEdit();

private:
    Ui::DiverEdit *ui;
};


} // namespace gui

