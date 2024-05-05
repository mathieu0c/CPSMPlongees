#include "MainWindow.hpp"

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <DBUtils.hpp>
#include <ProgramInterrupts.hpp>
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

  // const auto kLoadDbSuccess{cpsm::db::InitDB<false, false>(cpsm::consts::kCPSMDbPath)};
  const auto kLoadDbSuccess{cpsm::db::InitDB<true, true>(cpsm::consts::kCPSMDbPath)};
  if (!kLoadDbSuccess) {
    CPSM_ABORT_FOR(this, cpsm::AbortReason::kCouldNotInitDB);
  }
  ui->pg_editDiver->OnDatabaseLoaded();

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

  const auto kTmpDiver{db::GetDiverFromId(db::Def(), {5})};
  if (!kTmpDiver) {
    SPDLOG_ERROR("Failed to retrieve diver with id=5");
  }
  SPDLOG_INFO("Retrieved diver: {}", kTmpDiver.value());

  const auto kDiveCount{db::GetDiverDiveCount(kTmpDiver.value())};
  ui->pg_editDiver->SetDiver(kTmpDiver.value(), kDiveCount);

  connect(ui->pg_editDiver, &gui::DiverEdit::DiverEdited, this, &MainWindow::OnDiverEdited);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::OnDiverEdited(std::optional<std::tuple<db::Diver, db::DiverAddress>> edit_opt) {
  if (!edit_opt) {
    SPDLOG_DEBUG("Diver edition cancelled");
    return;
  }

  const auto &[diver, address]{edit_opt.value()};
  auto database{db::Def()};

  const auto kStoreResult{db::StoreDiverAndItsAddress(diver, address)};
  if (kStoreResult) {
    return; /* Everything was fine */
  }
  /* else */

  QMessageBox::critical(this,
                        tr("Erreur"),
                        tr("Impossible de sauvegarder les modifications apportées à %0 %1\n(Contacter le support si "
                           "besoin: ErrCode=<%2>)")
                            .arg(diver.last_name, diver.first_name)
                            .arg(kStoreResult.err_code));
  SPDLOG_ERROR("Failed to save driver: <{}> and address <{}> to database.\nErrCode=<{}>\nError: <{}>",
               diver,
               address,
               kStoreResult.err_code,
               database.lastError());

  using ErrCode = db::StoreDiverAndAddressResult::ErrCode;
  switch (kStoreResult.err_code) {
    case ErrCode::kFailedToRollback: {
      CPSM_ABORT_FOR(this, cpsm::AbortReason::kCouldNotRollback);
      break;
    }
    default: {
    }
  }
}

void MainWindow::on_action_check_updates_triggered() {
  m_updateHandler->show();
}
