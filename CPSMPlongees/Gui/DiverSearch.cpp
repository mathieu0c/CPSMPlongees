#include "DiverSearch.hpp"
#include "ui_DiverSearch.h"

namespace gui {

DiverSearch::DiverSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiverSearch)
{
    ui->setupUi(this);
}

DiverSearch::~DiverSearch()
{
    delete ui;
}

} // namespace gui
