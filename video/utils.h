#pragma once

#include <chrono>
#include <opencv2/core/mat.hpp>
#include <string>

template <typename clock_t = std::chrono::steady_clock,
          typename duration_t = std::chrono::milliseconds,
          typename timep_t = std::chrono::time_point<clock_t, duration_t>>
std::tuple<timep_t, duration_t> now_time_since(timep_t const& start) {
    auto now = clock_t::now();
    return {now, std::chrono::duration_cast<duration_t>(now - start)};
}

inline void expandBox(cv::Rect& box, float factor, cv::Rect bound) {
    auto delta_x = static_cast<int>(box.width * factor);
    auto delta_y = static_cast<int>(box.height * factor);
    box.x -= delta_x;
    box.y -= delta_y;
    box.width += delta_x * 2;
    box.height += delta_y * 2;

    if (box.x < bound.x) {
        box.x = bound.x;
    }
    if (box.y < bound.y) {
        box.y = bound.y;
    }
    if (box.x + box.width > bound.x + bound.width) {
        box.width = bound.x + bound.width - box.x;
    }
    if (box.y + box.height > bound.y + bound.height) {
        box.height = bound.y + bound.height - box.y;
    }
}
