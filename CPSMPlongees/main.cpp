#include <QApplication>
#include <QFontDatabase>
#include <QLocale>
#include <QTranslator>

#include <Logger/logger.hpp>
#include <Logger/logger_setup.hpp>

#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(DBScripts);
  //  installCustomLogHandler(logHandler::GlobalLogInfo{.progLogFilePath = "CPSMPlongees.log", .progName =
  //  "CPSMPlongees"});
  const auto kLogPath{logger::GetLogFilePath(CMAKEMACRO_PROJECT_EXE)};
  logger::SetupLoggerRotating(kLogPath, 2);

  SPDLOG_INFO("\n\n\n - {} - Hello! Welcome to CPSM Gestion plongées\n\n", QDateTime::currentDateTime().toString());

  QApplication a(argc, argv);

  // QFontDatabase::addApplicationFont(":/fonts/LEMONMILK-Bold.otf");

  // const QFont kBaseFont{"LEMONMILK"};
  // QApplication::setFont(kBaseFont);

  MainWindow w;
  w.show();
  return a.exec();
}
