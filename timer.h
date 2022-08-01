#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
  public:
    Timer() { timepoint_ = std::chrono::steady_clock::now(); }

    ~Timer() { printf("Calculation took %fms\n", elapsedMs()); }

    inline void reset() { timepoint_ = std::chrono::steady_clock::now(); }

    inline double elapsedMs() const {
      auto t = std::chrono::steady_clock::now() - timepoint_;
      return (std::chrono::duration_cast<std::chrono::microseconds>(t).count()) / 1000.0;
    }

    inline double elapsedUs() const {
      auto t = std::chrono::steady_clock::now() - timepoint_;
      return (std::chrono::duration_cast<std::chrono::microseconds>(t).count());
    }

    inline unsigned long long elapsedNs() const {
      auto t = std::chrono::steady_clock::now() - timepoint_;
      return (std::chrono::duration_cast<std::chrono::nanoseconds>(t).count());
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> timepoint_;
};

#endif
