#include "MainWindow.hpp"

#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <DBUtils.hpp>
#include <ProgramInterrupts.hpp>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextBrowser>

#include <Logger/btype.hpp>
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
      m_updateHandler{new updt::UpdateHandler(cpsm::consts::kCurrentVersion, cpsm::consts::kProjectGithubRelease,
                                              cpsm::consts::kPublicVerifierKeyFile, true,
                                              cpsm::consts::kPostUpdateCommand, true, this)} {
  ui->setupUi(this);

  if (cpsm::consts::kIsBuiltAsMockup && !btype::HasDebInfo()) {
    QMessageBox::warning(
        this, tr("Attention"), tr("Version de développement. Aucune modification ne sera enregistrée."));
  }

  // const auto kLoadDbSuccess{cpsm::db::InitDB<false, false>(cpsm::consts::kCPSMDbPath)};
  ui->pg_editDiver->RefreshFromDB();
  ui->mainDiverSearch->RefreshFromDB();
  ui->mainDiveSearch->RefreshFromDB();

  if (!QFileInfo::exists(c_config_file)) {
    SPDLOG_INFO("Saving default config file: {}", c_config_file);
  }

  connect(ui->pg_editDiver, &gui::DiverEdit::DiverEdited, this, &MainWindow::OnDiverEdited);

  connect(ui->mainDiverSearch, &gui::DiverSearch::DoubleClickOnDiver, this, &MainWindow::EditDiver);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::EditDiver(const cpsm::DiverWithDiveCount &diver) {
  ui->statusbar->clearMessage();
  ui->pg_editDiver->SetDiver(diver.diver, diver.dive_count);
  ui->tab_divers->setCurrentIndex(DiverTabPages::kEditDiver);
}

void MainWindow::OnDiverEdited(std::optional<std::tuple<cpsm::db::Diver, cpsm::db::DiverAddress> > edit_opt) {
  if (!edit_opt) {
    ui->tab_divers->setCurrentIndex(DiverTabPages::kBrowseDivers);
    return;
  }

  const auto &[diver, address]{edit_opt.value()};
  auto database{db::Def()};

  const auto kStoreResult{cpsm::db::StoreDiverAndItsAddress(diver, address)};
  if (kStoreResult) {
    ui->mainDiverSearch->RefreshFromDB();
    ui->tab_divers->setCurrentIndex(DiverTabPages::kBrowseDivers);
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

  using ErrCode = cpsm::db::StoreDiverAndAddressResult::ErrCode;
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

void MainWindow::on_pb_editDiver_clicked() {
  const auto kSelectedDiverOpt{ui->mainDiverSearch->GetSelectedDiver()};
  if (!kSelectedDiverOpt) {
    ui->statusbar->showMessage(tr("Sélectionnez 1 unique plongeur pour l'éditer"), 60000);
    return;
  }
  EditDiver(kSelectedDiverOpt.value());
}

void MainWindow::on_pb_deleteDiver_clicked() {
  const auto kSelectedDivers{ui->mainDiverSearch->GetSelectedDivers()};

  if (kSelectedDivers.size() == 0) {
    ui->statusbar->showMessage(tr("Sélectionnez au moins 1 plongeur à supprimer"), 60000);
    return;
  }

  /* Ask confirmation in a dialog box displaying the divers to be deleted */
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Confirmation"));

  QVBoxLayout layout(&dialog);

  QLabel label(tr("Voulez-vous vraiment supprimer les plongeurs suivants ?"));
  layout.addWidget(&label);

  QTextBrowser diver_display;
  diver_display.setText([&kSelectedDivers]() {
    QString out{};
    for (const auto &diver : kSelectedDivers) {
      out.append(tr("<%0 %1>\n").arg(diver.diver.last_name, diver.diver.first_name));
    }
    return out;
  }());
  layout.addWidget(&diver_display);

  QLabel warning(tr("Cette action est irréversible."));
  layout.addWidget(&warning);

  QDialogButtonBox buttonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, &dialog);
  buttonBox.setCenterButtons(true);
  QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  layout.addWidget(&buttonBox);

  if (dialog.exec() == QDialog::Rejected) {
    return;
  }

  bool success{true};
  auto database{db::Def()};
  if (!database.transaction()) {
    success = false;
  }

  for (const auto &diver : kSelectedDivers) {
    if (!cpsm::db::DeleteDiver(database, diver.diver)) {
      SPDLOG_WARN("Failed to delete diver: {}", diver.diver);
      success = false;
      break;
    }
  }
  if (database.commit()) {
    success = true;
  }

  if (!success) {
    if (!database.rollback()) {
      SPDLOG_ERROR("Failed to rollback transaction");
      CPSM_ABORT_FOR(this, cpsm::AbortReason::kCouldNotRollback);
    }
    SPDLOG_ERROR("Failed to remove divers from database: {}", database.lastError());
    /* Display to the user the list of divers that could not be deleted in an error message box */
    QMessageBox::critical(this, tr("Erreur"), tr("Impossible de supprimer les plongeurs sélectionnés"));
    return;
  }
  ui->statusbar->showMessage(
      tr("Suppression de %0 plongeur%1 réussie").arg(kSelectedDivers.size()).arg(kSelectedDivers.size() > 1 ? "s" : ""),
      60000);
  ui->mainDiverSearch->RefreshFromDB();
}

void MainWindow::on_pb_newDiver_clicked() {
  cpsm::DiverWithDiveCount default_diver{};
  default_diver.diver.diver_level_id = 1;
  default_diver.diver.birth_date = QDate::currentDate().addYears(-30);
  default_diver.diver.certif_date = QDate::currentDate();
  default_diver.diver.registration_date = QDate::currentDate();
  default_diver.diver.member_date = cpsm::consts::kEpochDate;
  EditDiver(default_diver);
}
