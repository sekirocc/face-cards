#pragma once

#include <chrono>
#include <string>

template <typename clock_t = std::chrono::steady_clock,
          typename duration_t = std::chrono::milliseconds,
          typename timep_t = std::chrono::time_point<clock_t, duration_t>>
std::tuple<timep_t, duration_t> now_time_since(timep_t const& start) {
    auto now = clock_t::now();
    return {now, std::chrono::duration_cast<duration_t>(now - start)};
}
