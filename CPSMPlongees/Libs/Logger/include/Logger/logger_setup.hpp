#pragma once

#include <src/constants.hpp>

// include separators

#include <QDebug>
#include <QFileInfo>
#include <QLoggingCategory>
#include <iostream>

#include <Logger/logger.hpp>
#include <Logger/qstring_callback_sink.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logger {

inline void CustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  const QString kFullFile{context.file ? context.file : ""};
  const QString kProjectDir{CMAKEMACRO_BASE_PROJECT_DIR};
  const QString kQrcStart{"qrc:/qt"};
  const auto kFullSize{kFullFile.size()};
  const QString file{kFullFile.startsWith(kQrcStart) && kFullSize > kQrcStart.size()
                         ? kFullFile.sliced(kQrcStart.size())
                         : (kFullSize > kProjectDir.size() ? kFullFile.sliced(kProjectDir.size() + 1) : kFullFile)};
  const char *function = context.function ? context.function : "";

  auto logger{spdlog::get(kLoggerName)};

  switch (type) {
    case QtDebugMsg:
      logger->debug("[{}:{} ({})]: {}", file, context.line, function, msg);
      break;
    case QtInfoMsg:
      logger->info("[{}:{} ({})]: {}", file, context.line, function, msg);
      break;
    case QtWarningMsg:
      logger->warn("[{}:{} ({})]: {}", file, context.line, function, msg);
      break;
    case QtCriticalMsg:
      logger->error("[{}:{} ({})]: {}", file, context.line, function, msg);
      break;
    case QtFatalMsg:
      logger->critical("[{}:{} ({})]: {}", file, context.line, function, msg);
      break;
  }
}

inline void InstallCustomLogHandler() {
  if (MaxLogLevel() == spdlog::level::debug) {
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
  } else {
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, false);
  }

  qInstallMessageHandler(CustomMessageHandler);
}

inline auto SetupLoggerRotating(const QString &q_log_path, int32_t flush_interval_s) {
  std::string log_path{q_log_path.toStdString()};
  std::vector<spdlog::sink_ptr> sinks;

  // Console logger
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(MaxLogLevel());
  //  console_sink->set_color_mode(spdlog::color_mode::always);
  sinks.push_back(console_sink);

  // File logger
  if (!log_path.empty()) {
    // 100Mo unique file
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_path, 100 * 1024 * 1024, 1, false);
    file_sink->set_level(MaxLogLevel());
    sinks.push_back(file_sink);
  } else {
    std::cerr << "WATCH OUT! No file logger was setup. Log will only output in "
                 "console.\n";
  }

  // Global logger
  auto logger = std::make_shared<spdlog::logger>(kLoggerName, begin(sinks), end(sinks));
  spdlog::drop(kLoggerName);  // Remove previous logger
  spdlog::register_logger(logger);
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern(kLoggerPattern);
  spdlog::flush_every(std::chrono::seconds(flush_interval_s));
  SPDLOG_DEBUG("Logger setup with pattern: {}", kLoggerPattern);

  InstallCustomLogHandler();

  SPDLOG_INFO("Logging to {}", log_path);

  return nullptr;
}

inline void StopLogger() {
  auto logger{spdlog::get(kLoggerName)};
  logger->flush();
}

}  // namespace logger
