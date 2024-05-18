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

  QApplication::setApplicationName("CPSMPlongees");
  QApplication::setApplicationVersion(to_string(consts::kCurrentVersion));

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

  // QFontDatabase::addApplicationFont(":/fonts/LEMONMILK-Bold.otf");

  // const QFont kBaseFont{"LEMONMILK"};
  // QApplication::setFont(kBaseFont);

  MainWindow w;
  w.show();
  return a.exec();
}
