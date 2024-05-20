#pragma once

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QStyle>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <string>

#include <Logger/logger.hpp>

namespace cpsm {

#define AbortReason_ENUM_VALS(name, value, user_msg, log_detail) name = value,
#define AbortReason_LIST(FUNC)                                                                                        \
  FUNC(kUnknown, 0, "Raison inconnue - contacter les développeurs", "unknown crash reason")                           \
  /*******************************/                                                                                   \
  /*    DB transactions errors   */                                                                                   \
  /*******************************/                                                                                   \
  FUNC(kCouldNotInitTransaction,                                                                                      \
       1,                                                                                                             \
       "Impossible d'initialiser une transaction dans la base de donnée",                                             \
       "Could not init db transaction")                                                                               \
  FUNC(kCouldNotRollback, 2, "Impossible de restaurer la base de données.", "Could not rollback transaction")         \
  FUNC(kCouldNotCommit,                                                                                               \
       3,                                                                                                             \
       "Impossible d'e restaurer'enregistrer les modifications de la base de données.",                               \
       "Could not commit transaction")                                                                                \
  /*******************************/                                                                                   \
  /* Database information errors */                                                                                   \
  /*******************************/                                                                                   \
  FUNC(kCouldNotGetDBInfo,                                                                                            \
       10,                                                                                                            \
       "Impossible de charger les informations de la base de donnée",                                                 \
       "Could not read db info such as version and date. This may lead to bad migration & corruption")                \
  FUNC(kCouldNotUpdateDBInfo,                                                                                         \
       11,                                                                                                            \
       "Impossible de mettre à jour les informations de la base de donnée",                                           \
       "Could not update db info such as version and date. This may lead to bad migration & corruption for the next " \
       "time")                                                                                                        \
  FUNC(kUnknownDBMigrationNeed,                                                                                       \
       12,                                                                                                            \
       "Erreur de développeur - Type de migration de base de donnée inconnu",                                         \
       "Unknown db migration need type")                                                                              \
  FUNC(kDBMigrationFailed,                                                                                            \
       13,                                                                                                            \
       "Impossible de migrer la base de donnée. Voir les logs pour plus d'informations.",                             \
       "Failed to upgrade (migrate) database")                                                                        \
  /*******************************/                                                                                   \
  /*          DB errors          */                                                                                   \
  /*******************************/                                                                                   \
  FUNC(kCouldNotInitDB, 20, "Impossible de charger la base de donnée", "Could not init db")

enum class AbortReason { AbortReason_LIST(AbortReason_ENUM_VALS) };

#define AbortReason_GET_USER_MSG(name, value, user_msg, log_detail) \
  case AbortReason::name:                                           \
    return user_msg;
inline QString GetUserMessage(AbortReason reason) {
  switch (reason) { AbortReason_LIST(AbortReason_GET_USER_MSG) }
  SPDLOG_ERROR("Unknown AbortReason: {}", static_cast<int>(reason));
  return {};
}

#define AbortReason_GET_LOG_MSG(name, value, user_msg, log_detail) \
  case AbortReason::name:                                          \
    return log_detail;
inline std::string GetLogMessage(AbortReason reason) {
  switch (reason) { AbortReason_LIST(AbortReason_GET_LOG_MSG) }
  SPDLOG_ERROR("Unknown AbortReason: {}", static_cast<int>(reason));
  return {};
}

#define AbortReason_GET_ENUM_VAL_NAME(name, value, user_msg, log_detail) \
  case AbortReason::name:                                                \
    return #name;
inline std::string GetAbortReasonEnumValueName(AbortReason reason) {
  switch (reason) { AbortReason_LIST(AbortReason_GET_ENUM_VAL_NAME) }
  SPDLOG_ERROR("Unknown AbortReason: {}", static_cast<int>(reason));
  return {};
}

#define CPSM_ABORT_FOR(parent, reason) AbortFor(parent, reason, __FILE__, __LINE__)
#define CPSM_ABORT_IF_FOR(condition, parent, reason) \
  if (condition) {                                   \
    CPSM_ABORT_FOR(parent, reason);                  \
  }

inline void AbortFor(QWidget* parent, AbortReason reason, const std::string& file, int line) {
  SPDLOG_ERROR(
      "We are in big trouble now ({}:{}). Actually, we should never ends up here.\n"
      "Aborting program for the following reason: {}\n"
      "Error n°: <{}> - <{}>",
      file,
      line,
      GetLogMessage(reason),
      static_cast<int>(reason),
      GetAbortReasonEnumValueName(reason));

  QDialog dialog(parent);
  dialog.setWindowTitle(QObject::tr("Erreur fatale"));

  auto* global_layout(new QVBoxLayout{&dialog});
  auto* global_hlayout(new QHBoxLayout{});
  global_layout->addLayout(global_hlayout);

  auto* icon_label = new QLabel{&dialog};
  QPixmap pixmap{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxCritical).pixmap(100, 100)};
  icon_label->setPixmap(pixmap);
  global_hlayout->addWidget(icon_label);

  auto* layout{new QVBoxLayout{}};
  global_hlayout->addLayout(layout);

  QLabel label(QObject::tr("Le programme a rencontré une erreur et va se fermer pour la raison suivante:"));
  layout->addWidget(&label);

  auto* error_display{new QTextBrowser{&dialog}};
  error_display->setText(
      QObject::tr("%2\n\nCode d'erreur: <%0> (<%1>)\nSource: %3:%4")
          .arg(static_cast<int>(reason))
          .arg(GetAbortReasonEnumValueName(reason).c_str(), cpsm::GetUserMessage(reason), file.c_str())
          .arg(line));
  layout->addWidget(error_display);

  QDialogButtonBox buttonBox(QDialogButtonBox::StandardButton::Ok, Qt::Horizontal, &dialog);
  buttonBox.setCenterButtons(true);
  QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  global_layout->addWidget(&buttonBox);

  dialog.exec();

  std::abort();
}

}  // namespace cpsm
