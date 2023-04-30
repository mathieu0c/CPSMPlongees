#pragma once

#include <QWidget>

namespace gui {

namespace Ui {
class DiveSearch;
}

class DiveSearch : public QWidget
{
    Q_OBJECT

public:
    explicit DiveSearch(QWidget *parent = nullptr);
    ~DiveSearch();

private:
    Ui::DiveSearch *ui;
};


} // namespace gui

