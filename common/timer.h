#pragma once

#include <chrono>

namespace common {

/// A flexible timer. TimeT is the precision of the timing, while scalingFactor
/// is the factor by which the output will be scaled. The default is to print
/// return milliseconds with microsecond precision.
template<typename TimeT = std::chrono::microseconds, int scalingFactor = 1000, typename ReturnType = double>
struct TimerT {
    typedef ReturnType return_type;
    TimerT() {
        reset();
    }

    void reset() {
        start = std::chrono::system_clock::now();
    }

    ReturnType get() const {
        TimeT duration = std::chrono::duration_cast<TimeT>(std::chrono::system_clock::now() - start);
        return (duration.count() * 1.0) / scalingFactor;
    }

    ReturnType getAndReset() {
        auto t = get();
        reset();
        return t;
    }

private:
    std::chrono::system_clock::time_point start;
};

/// A timer that is accurate to microseconds, formatted as milliseconds
typedef TimerT<std::chrono::microseconds, 1000, double> timer;
/// A timer that is accurate to milliseconds, formatted as seconds
typedef TimerT<std::chrono::milliseconds, 1000, double> sec_timer;

}
