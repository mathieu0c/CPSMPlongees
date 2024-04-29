#include "MainWindow.hpp"

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <DBUtils.hpp>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

#include <Logger/logger.hpp>

#include "./ui_MainWindow.h"

namespace {

void test() {}

}  // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      c_appdata_folder{cpsm::consts::kAppDataPath},
      c_config_file{QString{"%0/config.json"}.arg(c_appdata_folder)},
      m_updateHandler{new updt::UpdateHandler(consts::CURRENT_VERSION, consts::PROJECT_GITHUB_RELEASE,
                                              consts::PUBLIC_VERIFIER_KEY_FILE, true, consts::POST_UPDATE_CMD, true,
                                              this)},
      m_test_model{this} {
  ui->setupUi(this);

  cpsm::db::InitDB<true, true>(cpsm::consts::kCPSMDbPath);

  if (!QFileInfo::exists(c_config_file)) {
    SPDLOG_INFO("Saving default config file: {}", c_config_file);
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

  //  std::function<void(void)> nothing{[]() {}};
  //  const auto kWasUpdated{updt::acquireUpdated(nothing, consts::UPDATED_TAG_FILENAME)};
  //  SPDLOG_INFO("Was updated? {}", kWasUpdated);

  m_test_model.LoadFromDB();
  ui->tw_test->setModel(&m_test_model);

  const auto kTmpDiver{db::GetDiverFromId(db::Def(), {2})};
  ui->pg_editDiver->SetDiver(kTmpDiver.value());
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_action_check_updates_triggered() {
  m_updateHandler->show();
}
