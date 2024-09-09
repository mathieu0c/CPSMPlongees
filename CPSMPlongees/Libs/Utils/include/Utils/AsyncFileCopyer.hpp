#pragma once

#include <QFileInfo>
#include <QList>
#include <QMetaEnum>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <atomic>

#include <Logger/logger.hpp>

namespace utils {

template <typename QEnum>
std::string QtEnumToString(const QEnum value) {
  return std::string(QMetaEnum::fromType<QEnum>().valueToKey(static_cast<int>(value)));
}

/*
 * Will delete itself when the copy is done, after 5s
 * So use it with new and forget about it
 */
class AsyncFileCopyer : public QThread {
  Q_OBJECT

  static constexpr auto kDeleteDelayMs{5000};

 public:
  enum class ReturnCode : int {
    kSuccess,
    kFailed,
    kAborted,
    kDestinationFileExists,
    kFailedToOpenSource,
    kFailedToOpenDestination,
    kFailedToAbortProperly
  };
  Q_ENUM(ReturnCode)

 signals:
  void Progress(double progress);
  void Ended(ReturnCode success);

 public:
  AsyncFileCopyer(const QFileInfo& source, const QFileInfo& destination, bool override_destination = false,
                  bool auto_delete = true);
  ~AsyncFileCopyer() override {
    SPDLOG_DEBUG("Deleting AsyncFileCopyer");
  }

  bool WillNeedOverride() const {
    return m_destination.exists() && m_override_destination;
  }

  void Cancel();

  const auto& GetDestination() const {
    return m_destination;
  }

  const auto& GetSource() const {
    return m_source;
  }

 private:
  void run() override;

  bool OnCancelledOperations();

 private:
  QFileInfo m_source;
  QFileInfo m_destination;
  QFile m_destination_file;

  bool m_override_destination;
  bool m_display_copy_time{true};

  std::atomic_bool m_cancelled{false};
};

}  // namespace utils
