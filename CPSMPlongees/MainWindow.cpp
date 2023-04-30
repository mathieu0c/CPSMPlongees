#include "MainWindow.hpp"
#include "./ui_MainWindow.h"
#include <DBUtils.hpp>
#include <CPSMDatabase.hpp>

#include <CPSMGlobals.hpp>

#include <QStandardPaths>
#include <QFileInfo>
#include <QMessageBox>

#include <QDebug>

namespace{

void test(){

}

}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    c_appdata_folder{cpsm::consts::kAppDataPath},
      c_config_file{QString{"%0/config.json"}.arg(c_appdata_folder)},
      m_updateHandler{new updt::UpdateHandler(consts::CURRENT_VERSION,consts::PROJECT_GITHUB_RELEASE,
                                              consts::PUBLIC_VERIFIER_KEY_FILE,true,consts::POST_UPDATE_CMD,
                                              true,this)}
{
    ui->setupUi(this);

    cpsm::db::InitDB(cpsm::consts::kCPSMDbPath);

    if(!QFileInfo::exists(c_config_file)){
        qInfo() << "Saving default keyboard profile";
    }

    test();

//    const auto kPbOpt{pb::ReadFromFile<sot::KeyboardProfile>(c_config_file)};
//    if(!kPbOpt && false){
//        QString err{tr("Could not read config file: %0").arg(c_config_file)};
//        qCritical() << err;
//        QMessageBox::critical(this,tr("Error"),tr("Fatal error:\n%0").arg(err));
//        throw std::runtime_error(err.toStdString());
//    }
//    GetCurrentProfile() = kPbOpt.value();

    std::function<void(void)> nothing{[](){}};
    const auto kWasUpdated{updt::acquireUpdated(nothing,consts::UPDATED_TAG_FILENAME)};
    qInfo() << "Was updated?" << kWasUpdated;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_action_check_updates_triggered()
{
    m_updateHandler->show();
}

