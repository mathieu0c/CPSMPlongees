#pragma once

/*!
 * This file contains every utility function used to create database saves
 */

#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QThread>

#include <Logger/logger.hpp>

namespace db {

constexpr auto kDBBackupExtension{".db.backup"};

struct DBSaverConfiguration {
  QFileInfo db_path;
  QDir target_folder;
};

using ExistingBackupList = std::map<QDateTime, QString>;
// using ExistingBackupList = std::map<QDateTime, QString, std::greater<QDateTime>>; /**/

bool CheckConditionsForDBBackup(const DBSaverConfiguration& config);

ExistingBackupList SearchBackupFiles(const DBSaverConfiguration& config);
bool DBNeedsBackup(const ExistingBackupList& existing_backups);

/* Return array of QString of paths of the backups to remove and a bool indicating whether a new backup is needed */
std::tuple<std::vector<QString>, bool> GetBackupsUpdate(const ExistingBackupList& existingBackups);

QString GetNewBackupFileName(const QDateTime& date);
QString GetNewBackupFilePath(const DBSaverConfiguration& config, const QDateTime& date);

}  // namespace db
