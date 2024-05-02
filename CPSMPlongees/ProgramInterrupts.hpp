#pragma once

#include <QMessageBox>
#include <string>

#include <Logger/logger.hpp>

namespace cpsm {

#define AbortReason_ENUM_VALS(name, value, user_msg, log_detail) name = value,
#define AbortReason_LIST(FUNC)                                                                                \
  FUNC(kUnknown, 0, "Raison inconnue", "unknown crash reason")                                                \
  FUNC(kCouldNotRollback, 1, "Impossible de restaurer la base de données.", "Could not rollback transaction") \
  FUNC(kCouldNotInitDB, 2, "Impossible de charger la base de donnée", "Could not init db")

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

#define CPSM_ABORT_FOR(parent, reason) AbortFor(parent, reason, __FILE__, __LINE__)

inline void AbortFor(QWidget* parent, AbortReason reason, const std::string& file, int line) {
  SPDLOG_ERROR(
      "We are in big trouble now ({}:{}). Actually, we should never ends up here.\n"
      "Aborting program for the following reason: {}",
      file,
      line,
      GetLogMessage(reason));
  QMessageBox::critical(
      parent,
      QObject::tr("Erreur fatale"),
      QObject::tr(
          "Le programme a rencontré une erreur et va se fermer pour la raison suivate:\nCode d'erreur: <%0>\n%1")
          .arg(static_cast<int>(reason))
          .arg(cpsm::GetUserMessage(reason)));
  std::abort();
}

}  // namespace cpsm
