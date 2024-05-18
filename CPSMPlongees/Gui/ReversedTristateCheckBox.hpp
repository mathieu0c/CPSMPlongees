#pragma once

#include <QCheckBox>
#include <QObject>

class ReversedTristateCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    ReversedTristateCheckBox(QWidget *parent = nullptr);
    ReversedTristateCheckBox(const QString &text, QWidget *parent = nullptr);

protected:
    void nextCheckState() override;
};
