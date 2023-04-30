#include "DiverEdit.hpp"
#include "ui_DiverEdit.h"

namespace gui {

DiverEdit::DiverEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiverEdit)
{
    ui->setupUi(this);
}

DiverEdit::~DiverEdit()
{
    delete ui;
}

} // namespace gui
