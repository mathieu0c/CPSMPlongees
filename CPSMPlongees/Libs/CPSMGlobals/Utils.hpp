#pragma once

#include <Constants.hpp>
#include <QDateTime>

namespace cpsm {

inline bool IsMorning(const QTime &dt) {
  return dt.hour() < 12;
}

inline bool IsMorning(const QDateTime &dt) {
  return IsMorning(dt.time());
}

inline bool IsSunset(const QTime &dt) {
  return dt.hour() >= 20;
}

inline bool IsNight(const QTime &dt) {
  return dt.hour() >= 23;
}

inline QColor BackgroundColorFromTime(const QTime &time) {
  if (IsNight(time)) {
    return ::consts::colors::kBackgroundNightPurple;
  } else if (IsSunset(time)) {
    return ::consts::colors::kBackgroundSunsetOrange;
  } else if (IsMorning(time)) {
    return ::consts::colors::kBackgroundBlue;
  } else {
    return ::consts::colors::kBackgroundYellow;
  }
}

}  // namespace cpsm
