#pragma once

#include <QWidget>

namespace gui {

namespace Ui {
class DiverSearch;
}

class DiverSearch : public QWidget
{
    Q_OBJECT

public:
    explicit DiverSearch(QWidget *parent = nullptr);
    ~DiverSearch();

private:
    Ui::DiverSearch *ui;
};


} // namespace gui

