#pragma once

#include <QDialog>
#include <QStringList>

namespace gui {

namespace Ui {
class DialogConfirmListDeletion;
}

class DialogConfirmListDeletion : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfirmListDeletion(QWidget *parent = nullptr);
    ~DialogConfirmListDeletion();

    void SetElementList(const QStringList& new_elements);
    void SetTextLabel(const QString& new_text);

    static bool ConfirmDeletion(const QString& title, const QString& text_label, QStringList elements, const bool kSort = true);

private:
    Ui::DialogConfirmListDeletion *ui;
};


} // namespace gui
