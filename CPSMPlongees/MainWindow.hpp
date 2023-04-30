#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <Constants.hpp>

#include <UpdateHandler.hpp>
#include <QMainWindow>
#include <QPushButton>

#include <functional>
#include <unordered_map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    const QColor kBaseBlueColor{25,180,255};

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_action_check_updates_triggered();

private:

private:
    Ui::MainWindow *ui;

    const QString c_appdata_folder;
    const QString c_config_file;

    updt::UpdateHandler* m_updateHandler;

//    sot::KeyboardProfile m_config{};
};
#endif // MAINWINDOW_HPP
