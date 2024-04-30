#pragma once

#include <src/constants.hpp>

// include separator

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

// template <typename T>
// concept ElementIterable = std::ranges::range<std::ranges::range_value_t<T>>;
template <typename T>
concept ElementIterable = requires(std::ranges::range_value_t<T> x) {
  x.begin();  // must have `x.begin()`
  x.end();    // and `x.end()`
};

/*!
 * Declare a function protype with std::ostream "stream" and TypeName& "val"
 */
#define SETUP_SPD_FORMATTER(TypeName) \
  template <>                         \
  struct fmt::formatter<TypeName> : ostream_formatter {};

#define DECLARE_CUSTOM_SPD_FORMAT(TypeName) \
  SETUP_SPD_FORMATTER(TypeName)             \
  inline std::ostream &operator<<(std::ostream &stream, const TypeName &val)

DECLARE_CUSTOM_SPD_FORMAT(QString) {
  return stream << val.toStdString();
}

DECLARE_CUSTOM_SPD_FORMAT(QFileInfo) {
  return stream << val.absoluteFilePath();
}

template <typename CIter>
std::string ArrayToString(CIter begin, CIter end) {
  std::string str{"{"};
  for (auto it = begin; it != end; ++it) {
    fmt::format_to(std::back_inserter(str), "{}, ", *it);
  }
  if (std::distance(begin, end) > 0) {
    str.pop_back();
    str.pop_back();
  }
  str.append("}");
  return str;
}
template <ElementIterable Iterable>
std::string ArrayToString(const Iterable &val) {
  return ArrayToString(val.cbegin(), val.cend());
}

template <ElementIterable Iterable>
inline std::ostream &operator<<(std::ostream &os, const Iterable &val) {
  return os << ArrayToString(val.cbegin(), val.cend());
}

namespace logger {
inline QString GetLogFolderPath() {
  auto val{QString{"%0/Logs"}.arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))};
  return val;
}

inline QString GetLogFilePath(const QString &prog_name) {
  auto val{QString{"%0/%1Logs.txt"}.arg(GetLogFolderPath(), prog_name)};
  return val;
}

}  // namespace logger
