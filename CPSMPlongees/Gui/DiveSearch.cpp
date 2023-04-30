#include "DiveSearch.hpp"
#include "ui_DiveSearch.h"

namespace gui {

DiveSearch::DiveSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiveSearch)
{
    ui->setupUi(this);
}

DiveSearch::~DiveSearch()
{
    delete ui;
}

} // namespace gui
