#include <CPSMDatabase.hpp>
#include <CPSMGlobals.hpp>
#include <DBUtils.hpp>
#include <QApplication>
#include <QFontDatabase>
#include <QLocale>
#include <QTranslator>

#include <Logger/logger.hpp>
#include <Logger/logger_setup.hpp>

#include "Constants.hpp"
#include "MainWindow.hpp"
#include "ProgramInterrupts.hpp"

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(DBScripts);

  QApplication::setApplicationName(cpsm::consts::kAppName);
  QApplication::setApplicationVersion(to_string(cpsm::consts::kCurrentVersion));

  //  installCustomLogHandler(logHandler::GlobalLogInfo{.progLogFilePath = "CPSMPlongees.log", .progName =
  //  "CPSMPlongees"});
  const auto kLogPath{logger::GetLogFilePath(CMAKEMACRO_PROJECT_EXE)};
  logger::SetupLoggerRotating(kLogPath, 2);

  SPDLOG_INFO("\n\n\n - {} - Hello! Welcome to CPSM Gestion plongées\n\n", QDateTime::currentDateTime().toString());

  QApplication a(argc, argv);

  const auto kLoadDbSuccess{cpsm::db::InitDB<true, true>(cpsm::consts::kCPSMDbPath)};
  if (!kLoadDbSuccess) {
    CPSM_ABORT_FOR(nullptr, cpsm::AbortReason::kCouldNotInitDB);
  }

  const auto kDBInfoOpt{cpsm::db::GetDBInfoFromId(db::Def(), {1})};
  if (!kDBInfoOpt) {
    CPSM_ABORT_FOR(nullptr, cpsm::AbortReason::kCouldNotGetDBInfo);
  }
  auto db_info{*kDBInfoOpt};
  SPDLOG_INFO("");
  SPDLOG_INFO("DB infos read: {}", db_info);

  const auto kLastDBSoftVersionOpt{updt::Version::FromString(db_info.latest_cpsm_soft_version_used)};
  if (!kLastDBSoftVersionOpt) {
    CPSM_ABORT_FOR(nullptr, cpsm::AbortReason::kCouldNotGetDBInfo);
  }
  const auto kLastDBSoftVersion{*kLastDBSoftVersionOpt};
  if (/*kLastDBSoftVersion != updt::Version{0, 0, 0} &&*/ kLastDBSoftVersion != cpsm::consts::kCurrentVersion) {
    SPDLOG_INFO("We may need a db migration here? last used soft version <{}> to current version <{}>",
                kLastDBSoftVersion,
                cpsm::consts::kCurrentVersion);
  }

  db_info.latest_cpsm_soft_version_used = to_string(cpsm::consts::kCurrentVersion);
  db_info.latest_cpsm_db_version_used = to_string(cpsm::consts::kCurrentVersion);
  db_info.latest_open_date = QDateTime::currentDateTime();
  SPDLOG_INFO("Updating DB infos to {}", db_info);
  const auto kUpdateInfoSuccess{cpsm::db::UpdateDBInfo(db::Def(), db_info)};
  SPDLOG_INFO("Update db info success? {}", kUpdateInfoSuccess.has_value());
  if (!kUpdateInfoSuccess) {
    CPSM_ABORT_FOR(nullptr, cpsm::AbortReason::kCouldNotGetDBInfo);
  }
  SPDLOG_INFO("");

  // QFontDatabase::addApplicationFont(":/fonts/LEMONMILK-Bold.otf");

  // const QFont kBaseFont{"LEMONMILK"};
  // QApplication::setFont(kBaseFont);

  MainWindow w;
  w.show();
  return a.exec();
}
