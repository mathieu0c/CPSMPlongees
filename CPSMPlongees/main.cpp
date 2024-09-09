#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <DBMigration.hpp>
#include <DBUtils.hpp>
#include <QApplication>
#include <QFontDatabase>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

#include <Logger/logger.hpp>
#include <Logger/logger_setup.hpp>
#include <Utils/AsyncFileCopyer.hpp>

#include "DBSaver.hpp"
#include "MainWindow.hpp"
#include "ProgramInterrupts.hpp"

bool BlockingDBBackup(const QString& original_file, const QString& destination_file) {
  std::atomic<utils::AsyncFileCopyer::ReturnCode> copy_result{utils::AsyncFileCopyer::ReturnCode::kFailed};
  std::atomic_bool can_exit_this_function{false};

  utils::AsyncFileCopyer copyer{QFileInfo{original_file}, QFileInfo{destination_file}, true, false};
  SPDLOG_INFO("Making backup from <{}> to <{}>",
              copyer.GetSource().absoluteFilePath(),
              copyer.GetDestination().absoluteFilePath());

  QProgressDialog progress{
      QObject::tr("Sauvegarde en cours de la base de données"), QObject::tr("Annuler la sauvegarde"), 0, 1000};
  QObject::connect(&progress, &QProgressDialog::canceled, &copyer, [&copyer]() { copyer.Cancel(); });

  progress.setWindowModality(Qt::WindowModal);
  QObject::connect(&copyer, &utils::AsyncFileCopyer::Progress, &copyer, [&progress](double progress_val) {
    progress.setValue(int(progress_val * 1000));
  });
  QObject::connect(&copyer,
                   &utils::AsyncFileCopyer::Ended,
                   &copyer,
                   [&progress, &copy_result, &can_exit_this_function](utils::AsyncFileCopyer::ReturnCode success) {
                     copy_result = success;
                     progress.close();
                     can_exit_this_function = true;
                   });

  progress.show();
  copyer.start();

  while (!can_exit_this_function) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    QCoreApplication::processEvents();
  }

  const utils::AsyncFileCopyer::ReturnCode kDisplayResult{copy_result};
  SPDLOG_INFO("Copy done with success: {}", utils::QtEnumToString(kDisplayResult));

  return copy_result == utils::AsyncFileCopyer::ReturnCode::kSuccess ||
         copy_result == utils::AsyncFileCopyer::ReturnCode::kAborted;
}

bool RemoveFileList(const std::vector<QString>& to_remove) {
  SPDLOG_INFO("Removing {} old backups", to_remove.size());
  bool success{true};
  for (const auto& path : to_remove) {
    if (!QFile::remove(path)) {
      SPDLOG_ERROR("Failed to remove file: {}", path);
      success = false;
    } else {
      SPDLOG_INFO("\tRemoved {}", path);
    }
  }
  return success;
}

int main(int argc, char* argv[]) {
  /*#########################*/
  /*#                       #*/
  /*#       Base setup      #*/
  /*#                       #*/
  /*#########################*/

  Q_INIT_RESOURCE(DBScripts);

  QApplication::setApplicationName(cpsm::consts::kAppName);
  QApplication::setApplicationVersion(to_string(cpsm::consts::kCurrentVersion));

  const auto kLogPath{logger::GetLogFilePath(CMAKEMACRO_PROJECT_EXE)};
  logger::SetupLoggerRotating(kLogPath, 2);

  SPDLOG_INFO("\n\n\n - {} - Hello! Welcome to CPSM Gestion plongées\n\n", QDateTime::currentDateTime().toString());
  QApplication a(argc, argv);

  /*#########################*/
  /*#                       #*/
  /*#       DB Backup       #*/
  /*#                       #*/
  /*#########################*/

  const db::DBSaverConfiguration kDBSaverConfig{QFileInfo{cpsm::consts::kCPSMDbPath},
                                                QDir{cpsm::consts::kDBBackupPath}};

  SPDLOG_INFO("DB Backup configuration:\nDB_PATH={}\nBACKUP_FOLDER={}",
              kDBSaverConfig.db_path.absoluteFilePath(),
              kDBSaverConfig.target_folder.absolutePath());

  if (!db::CheckConditionsForDBBackup(kDBSaverConfig)) {
    QMessageBox::critical(nullptr,
                          QObject::tr("Erreur"),
                          QObject::tr("Erreur lors de la sauvegarde de la base de donnée (initialisation)"));
    SPDLOG_ERROR("Could not check conditions for DB backup");
  } else {
    SPDLOG_INFO("DB backup conditions checked");

    const auto kExistingBackups{db::SearchBackupFiles(kDBSaverConfig)};
    for (const auto& [date, path] : kExistingBackups) {
      SPDLOG_INFO("Found backup: {} - {}", date.toString(), path.toStdString());
    }

    const auto [kToRemove, kNeedBackup]{db::GetBackupsUpdate(kExistingBackups)};
    SPDLOG_INFO(
        "Need to remove {} backups. Is a new backup needed? <{}>", kToRemove.size(), kNeedBackup ? "yes" : "no");
    for (const auto& path : kToRemove) {
      SPDLOG_INFO("\tTo remove: {}", path);
    }

    if (kNeedBackup) {
      if (!BlockingDBBackup(kDBSaverConfig.db_path.absoluteFilePath(),
                            db::GetNewBackupFilePath(kDBSaverConfig, QDateTime::currentDateTime()))) {
        QMessageBox::critical(nullptr,
                              QObject::tr("Erreur"),
                              QObject::tr("Erreur lors de la sauvegarde de la base de donnée (copie du fichier)"));
        SPDLOG_ERROR("Failed to copy DB file for backup");
      } else { /* If a new backup is needed, only remove old ones IF the new one is successful */
        if (!RemoveFileList(kToRemove)) {
          SPDLOG_ERROR("Failed to remove some of the remaining backups...");
        }
      }
    } else {
      if (!RemoveFileList(kToRemove)) {
        SPDLOG_ERROR("Failed to remove some of the remaining backups...");
      }
    }
  }
  SPDLOG_INFO("DB Backup ends -------------\n\n");

  /*#########################*/
  /*#                       #*/
  /*#  DB load & migration  #*/
  /*#                       #*/
  /*#########################*/

  const auto kLoadDbSuccess{cpsm::db::InitDB<true, true>(cpsm::consts::kCPSMDbPath)};
  CPSM_ABORT_IF_FOR(!kLoadDbSuccess, nullptr, cpsm::AbortReason::kCouldNotInitDB);

  /* -- DB retrieving db information -- */

  auto database{db::Def()};

  const auto kDBInfoOpt{cpsm::db::GetDBInfoFromId(database, {1})};
  CPSM_ABORT_IF_FOR(!kDBInfoOpt, nullptr, cpsm::AbortReason::kCouldNotGetDBInfo);

  auto db_info{*kDBInfoOpt};
  SPDLOG_INFO("");
  SPDLOG_INFO("DB infos read: {}", db_info);

  const auto kLastDBSoftVersionOpt{updt::Version::FromString(db_info.latest_cpsm_soft_version_used)};
  CPSM_ABORT_IF_FOR(!kLastDBSoftVersionOpt, nullptr, cpsm::AbortReason::kCouldNotGetDBInfo);
  const auto kLastDBSoftVersion{*kLastDBSoftVersionOpt};

  /* -- DB actual Migration -- */

  CPSM_ABORT_IF_FOR(!database.transaction(), nullptr, cpsm::AbortReason::kCouldNotInitTransaction);

  namespace migration = cpsm::db::migration;
  const auto kNeedsMigration{migration::VersionNeedsMigration(kLastDBSoftVersion, cpsm::consts::kCurrentVersion)};
  switch (kNeedsMigration) {
    case migration::NeedsMigrationState::kNoMigrationNeeded:
      break;
    case migration::NeedsMigrationState::kUpgradeNeeded: /* We'll check that later for lisibility reason */
      break;
    case migration::NeedsMigrationState::kDowngradeNeeded: {
      const auto kShouldExecute{QMessageBox::critical(
          nullptr,
          QObject::tr("ATTENTION"),
          QObject::tr("La base de donnée a dernièrement été ouverte avec une version du logiciel plus récente (%0).\n"
                      "L'ouvrir avec cette version plus ancienne (%1) va probablement causer une corruption des "
                      "données irrémédiable.\n"
                      "\n"
                      "Souhaitez-vous malgré tout lancer le logiciel ?")
              .arg(to_string(kLastDBSoftVersion), to_string(cpsm::consts::kCurrentVersion)),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No)};
      if (kShouldExecute == QMessageBox::No) {
        return 0;
      }
      SPDLOG_WARN("User decided to open the software with a lower version ({}) than the one used last time ({})",
                  cpsm::consts::kCurrentVersion,
                  kLastDBSoftVersion);
      break;
    }
    default:
      SPDLOG_CRITICAL("Unknown migration need type: {}", static_cast<int>(kNeedsMigration));
      CPSM_ABORT_FOR(nullptr, cpsm::AbortReason::kUnknownDBMigrationNeed);
      break;
  }

  if (kNeedsMigration == migration::NeedsMigrationState::kUpgradeNeeded) {
    if (!migration::MigrateDB(kLastDBSoftVersion, cpsm::consts::kCurrentVersion, database)) {
      /* Make sure we rollback to prevent problems on next opening */
      CPSM_ABORT_IF_FOR(!database.rollback(), nullptr, cpsm::AbortReason::kCouldNotRollback);

      CPSM_ABORT_FOR(nullptr, cpsm::AbortReason::kDBMigrationFailed);
    }
  }

  db_info.latest_cpsm_soft_version_used = to_string(cpsm::consts::kCurrentVersion);
  db_info.latest_cpsm_db_version_used = to_string(cpsm::consts::kCurrentVersion);
  db_info.latest_open_date = QDateTime::currentDateTime();
  SPDLOG_INFO("Updating DB infos to {}", db_info);
  const auto kUpdateInfoSuccess{cpsm::db::UpdateDBInfo(database, db_info)};
  SPDLOG_INFO("Update db info success? {}", kUpdateInfoSuccess.has_value());
  if (!kUpdateInfoSuccess) {
    CPSM_ABORT_IF_FOR(!database.rollback(), nullptr, cpsm::AbortReason::kCouldNotRollback);
  }
  CPSM_ABORT_IF_FOR(!database.commit(), nullptr, cpsm::AbortReason::kCouldNotCommit);
  SPDLOG_INFO("");

  /*#########################*/
  /*#                       #*/
  /*#   Application start   #*/
  /*#                       #*/
  /*#########################*/

  // QFontDatabase::addApplicationFont(":/fonts/LEMONMILK-Bold.otf");

  // const QFont kBaseFont{"LEMONMILK"};
  // QApplication::setFont(kBaseFont);

  MainWindow w;
  w.show();
  return a.exec();
}
