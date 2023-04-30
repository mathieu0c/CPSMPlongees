#pragma once

#include <QWidget>

namespace gui {

namespace Ui {
class DiveEdit;
}

class DiveEdit : public QWidget
{
    Q_OBJECT

public:
    explicit DiveEdit(QWidget *parent = nullptr);
    ~DiveEdit();

private:
    Ui::DiveEdit *ui;
};


} // namespace gui
