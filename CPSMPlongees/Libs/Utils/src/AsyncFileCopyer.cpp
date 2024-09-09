#include "include/Utils/AsyncFileCopyer.hpp"

#include <Logger/logger.hpp>
#include <Utils/chrono.hpp>

namespace utils {

AsyncFileCopyer::AsyncFileCopyer(const QFileInfo& source, const QFileInfo& destination, bool override_destination,
                                 bool auto_delete)
    : m_source{source}, m_destination{destination}, m_override_destination{override_destination} {
  if (m_destination.isDir()) {
    m_destination = QFileInfo(m_destination.filePath() + "/" + m_source.fileName());
  }

  if (auto_delete) {
    connect(this, &AsyncFileCopyer::Ended, this, [this](ReturnCode) {
      QTimer::singleShot(kDeleteDelayMs, this, &QObject::deleteLater);
    });
  }
}

void AsyncFileCopyer::Cancel() {
  m_cancelled = true;
}

void AsyncFileCopyer::run() {
  Chrono chrono{};
  if (m_destination.exists() && !m_override_destination) {
    emit Ended(ReturnCode::kDestinationFileExists);
    return;
  }

  if (!m_source.exists()) {
    emit Ended(ReturnCode::kFailedToOpenSource);
    return;
  }

  QFile source_file{m_source.absoluteFilePath()};

  if (!source_file.open(QIODevice::ReadOnly)) {
    emit Ended(ReturnCode::kFailedToOpenSource);
    return;
  }

  m_destination_file.setFileName(m_destination.absoluteFilePath());

  if (!m_destination_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    emit Ended(ReturnCode::kFailedToOpenDestination);
    return;
  }

  const auto kSourceSize{m_source.size()};
  const auto kBufferSize{std::min(qint64(1024 * 1024), (kSourceSize / 100) + 1)}; /* 1 MB chunk or 1% */
  auto buffer = std::make_unique<char[]>(kBufferSize);

  qint64 total_bytes_copied{};

  auto bytes_read{0};
  while ((bytes_read = source_file.read(buffer.get(), kBufferSize)) > 0) {
    if (m_cancelled) {
      emit Ended(OnCancelledOperations() ? ReturnCode::kAborted : ReturnCode::kFailedToAbortProperly);
      return;
    }
    if (m_destination_file.write(buffer.get(), bytes_read) != bytes_read) {
      emit Ended(ReturnCode::kFailed);
      return;
    }
    total_bytes_copied += bytes_read;
    emit Progress(static_cast<double>(total_bytes_copied) / static_cast<double>(kSourceSize));
  }

  if (m_display_copy_time) {
    SPDLOG_INFO("Copy from <{}> to <{}> took {} ms", m_source, m_destination, chrono.Time());
  }

  emit AsyncFileCopyer::Ended(ReturnCode::kSuccess);
}

bool AsyncFileCopyer::OnCancelledOperations() {
  if (!m_destination_file.isOpen()) {
    return true;
  }

  return m_destination_file.remove();
}

}  // namespace utils
