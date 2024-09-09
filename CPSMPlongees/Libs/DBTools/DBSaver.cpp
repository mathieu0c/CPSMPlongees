#include "DBSaver.hpp"

#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <map>
#include <set>

#include "Database.hpp"

namespace {

constexpr auto kDBSaveDateTimeFormat{"yyyy-MM-dd_HH-mm-ss"};

}  // namespace

namespace db {

bool CheckConditionsForDBBackup(const DBSaverConfiguration& config) {
  if (!config.db_path.exists()) {
    SPDLOG_WARN("DB file not found: {}", config.db_path.filePath().toStdString());
    return false;
  }

  if (config.target_folder.exists()) {
    return true;
  }

  /* else */
  SPDLOG_INFO("Target folder does not exist: {}", config.target_folder.path().toStdString());
  if (config.target_folder.mkpath(".")) {
    SPDLOG_INFO("Created folder: <{}>", config.target_folder.absolutePath());
    return true;
  }
  SPDLOG_ERROR("Failed to create folder: <{}>", config.target_folder.absolutePath());
  return false;
}

ExistingBackupList SearchBackupFiles(const DBSaverConfiguration& config) {
  ExistingBackupList matchingFiles;
  QRegularExpression regex(R"(\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2})");

  QDirIterator it(config.target_folder.absolutePath(), QDir::Files, QDirIterator::NoIteratorFlags);

  while (it.hasNext()) {
    it.next();
    QFileInfo fileInfo = it.fileInfo();
    QRegularExpressionMatch match = regex.match(fileInfo.fileName());
    if (match.hasMatch()) {
      QString dateTimeString = match.captured(0);
      QDateTime dateTime = QDateTime::fromString(dateTimeString, kDBSaveDateTimeFormat);
      if (dateTime.isValid()) {
        matchingFiles[dateTime] = fileInfo.absoluteFilePath();
      }
    }
  }

  return matchingFiles;
}

std::tuple<std::vector<QString>, bool> GetBackupsUpdate(const ExistingBackupList& existingBackups) {
  bool needNewBackup{false};
  std::vector<QString> backupsToRemove;
  std::set<QDateTime> backupsToKeep;
  std::set<QDateTime> monthly_backups{};
  QDateTime now = QDateTime::currentDateTime();

  for (const auto& backup : existingBackups) {
    QDateTime backupDate = backup.first;
    int daysDiff = backupDate.daysTo(now);

    if (daysDiff <= 7) {
      backupsToKeep.insert(backupDate);
    } else if (daysDiff <= 30) {
      if (backupsToKeep.empty() || backupsToKeep.rbegin()->daysTo(backupDate) >= 7) {
        backupsToKeep.insert(backupDate);
      }
    } else if (daysDiff <= 365) {
      if (backupsToKeep.empty() || backupDate.date().day() == 1) {
        backupsToKeep.insert(backupDate);
      }
    }
  }

  for (const auto& backup : existingBackups) {
    if (backupsToKeep.find(backup.first) == backupsToKeep.end()) {
      backupsToRemove.push_back(backup.second);
    }
  }

  needNewBackup = backupsToKeep.empty() || backupsToKeep.rbegin()->daysTo(now) > 0;

  return {backupsToRemove, needNewBackup};
}

// void DBSaverThread::run() {
//   if (!m_config.db_path.exists()) {
//     SPDLOG_ERROR("DB file not found: {}",
//     m_config.db_path.filePath().toStdString()); emit
//     SaveDone(ExecReturnCode::kDbFileNotFound); return;
//   }

//   if (!m_config.target_folder.exists()) {
//     SPDLOG_ERROR("Target folder does not exist: {}",
//     m_config.target_folder.path().toStdString()); emit
//     SaveDone(ExecReturnCode::kInvalidSaveFolder); return;
//   }

//   const auto kDbFile{m_config.db_path.fileName()};
//   const auto kDbPath{m_config.db_path.filePath()};
//   const auto kDbName{m_config.db_path.baseName()};
//   const auto kDbSuffix{m_config.db_path.completeSuffix()};
//   const auto kDbDate{QDate::currentDate().toString("yyyy-MM-dd")};

//   const auto kSaveName{QString("%0_%1.%2").arg(kDbName, kDbDate, kDbSuffix)};
//   const auto kSavePath{m_config.target_folder.filePath(kSaveName)};

//   if (QFile::exists(kSavePath)) {
//     SPDLOG_INFO("Save already exists: {}", kSavePath.toStdString());
//     emit SaveDone(ExecReturnCode::kNoSaveNeeded);
//     return;
//   }

//   if (!QFile::copy(kDbPath, kSavePath)) {
//     SPDLOG_ERROR("Failed to copy db file to save location: {}",
//     kSavePath.toStdString()); emit SaveDone(ExecReturnCode::kFailed); return;
//   }

//   SPDLOG_INFO("Save created: {}", kSavePath.toStdString());
//   emit SaveDone(ExecReturnCode::kOk);
// }

QString GetNewBackupFileName(const QDateTime& date) {
  return date.toString(kDBSaveDateTimeFormat) + kDBBackupExtension;
}

QString GetNewBackupFilePath(const DBSaverConfiguration& config, const QDateTime& date) {
  return config.target_folder.filePath(GetNewBackupFileName(date));
}

}  // namespace db
