#pragma once

#include <chrono>

namespace utils {

template <typename TimeType = std::chrono::milliseconds>
class Chrono {
 public:
  Chrono() {
    Start();
  }
  void Start() {
    m_start_time = std::chrono::high_resolution_clock::now();
  }

  double Time() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<TimeType>(end_time - m_start_time);
    return static_cast<double>(duration.count());
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};

}  // namespace utils
