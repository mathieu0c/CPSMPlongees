#include "DialogConfirmListDeletion.hpp"
#include "ui_DialogConfirmListDeletion.h"

#include <algorithm>

namespace gui {

DialogConfirmListDeletion::DialogConfirmListDeletion(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfirmListDeletion)
{
    ui->setupUi(this);

    setModal(true);
}

DialogConfirmListDeletion::~DialogConfirmListDeletion()
{
    delete ui;
}

void DialogConfirmListDeletion::SetElementList(const QStringList& new_elements){
    ui->lw_elementList->clear();
    ui->lw_elementList->addItems(new_elements);
}
void DialogConfirmListDeletion::SetTextLabel(const QString& new_text){
    ui->lbl_title->setText(new_text);
}

bool DialogConfirmListDeletion::ConfirmDeletion(const QString& title, const QString &text_label, QStringList elements, const bool kSort){
    DialogConfirmListDeletion dial{};
    dial.setWindowTitle(title);

    dial.SetTextLabel(text_label);

    if(kSort){
        std::sort(elements.begin(),elements.end());
    }
    dial.SetElementList(elements);

    return dial.exec() == QDialog::Accepted;
}

} // namespace gui
