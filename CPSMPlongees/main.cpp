#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <DBMigration.hpp>
#include <DBUtils.hpp>
#include <QApplication>
#include <QFontDatabase>
#include <QLocale>
#include <QTranslator>

#include <Logger/logger.hpp>
#include <Logger/logger_setup.hpp>

#include "MainWindow.hpp"
#include "ProgramInterrupts.hpp"

int main(int argc, char *argv[]) {
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

  if (kNeedsMigration == migration::NeedsMigrationState::kUpgradeNeeded || true) {
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
