#include "DiveEdit.hpp"
#include "ui_DiveEdit.h"

namespace gui {

DiveEdit::DiveEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiveEdit)
{
    ui->setupUi(this);
}

DiveEdit::~DiveEdit()
{
    delete ui;
}

} // namespace gui
