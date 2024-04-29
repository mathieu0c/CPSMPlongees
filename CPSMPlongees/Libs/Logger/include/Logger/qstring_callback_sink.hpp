// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <QString>
#include <mutex>
#include <string>

#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>
// #include <spdlog/sinks/qt_sinks.h>

namespace spdlog {

// callbacks type
typedef std::function<void(const QString &msg)> custom_qstring_callback;

namespace sinks {
/*
 * Trivial callback sink, gets a callback function and calls it on each log
 */
template <typename Mutex>
class callback_qstring_sink final : public base_sink<Mutex> {
 public:
  static constexpr std::array kColors{"#aaeeff" /* trace */,
                                      "#00ccff" /* debug */,
                                      "#52ff42" /* info */,
                                      "#FF6600" /* warn */,
                                      "#D40000" /* err */,
                                      "#FF0000" /* critical */,
                                      "#FFFFFF" /* off */}; /*Check spdlog/common.h for levels values*/
  static_assert(kColors.size() == level::n_levels);

 public:
  explicit callback_qstring_sink(const custom_qstring_callback &callback, bool escape_for_html, bool use_colors)
      : callback_{callback}, m_escape_for_html{escape_for_html}, m_use_colors{use_colors} {}

  void set_callback(const custom_qstring_callback &callback) {
    callback_ = callback;
  }

 protected:
  void sink_it_(const details::log_msg &msg) override {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    const string_view_t str = string_view_t(formatted.data(), formatted.size());
    QString msg_str{QString::fromUtf8(str.data(), static_cast<int>(str.size())).trimmed()};
    if (m_escape_for_html) {
      msg_str = msg_str.toHtmlEscaped();
    }
    if (m_use_colors) {
      msg_str = QString{"%0<span style=\"font-weight:700;color:%1\">%2</span>%3"}.arg(
          msg_str.mid(0, msg.color_range_start),
          kColors.at(static_cast<qsizetype>(msg.level)),
          msg_str.mid(msg.color_range_start, msg.color_range_end - msg.color_range_start),
          msg_str.mid(msg.color_range_end));
    }
    callback_(msg_str + "\n\n");
  }
  void flush_() override{};

 private:
  custom_qstring_callback callback_;
  bool m_escape_for_html;
  bool m_use_colors;
};

using callback_qstring_sink_mt = callback_qstring_sink<std::mutex>;
using callback_qstring_sink_st = callback_qstring_sink<details::null_mutex>;

}  // namespace sinks

//
// factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> callback_logger_mt(const std::string &logger_name,
                                                  const custom_qstring_callback &callback) {
  return Factory::template create<sinks::callback_qstring_sink_mt>(logger_name, callback);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> callback_logger_st(const std::string &logger_name,
                                                  const custom_qstring_callback &callback) {
  return Factory::template create<sinks::callback_qstring_sink_st>(logger_name, callback);
}

}  // namespace spdlog
