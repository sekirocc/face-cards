#pragma once

#include <SDL_opengl.h>
#include <__chrono/duration.h>
#include <chrono>
#include <string>

bool LoadImageFromFile(const std::string& filename,
                       GLuint& out_texture,
                       int& out_width,
                       int& out_height,
                       std::vector<uint8_t>& out_image_data);

template <typename clock_t = std::chrono::steady_clock,
          typename duration_t = std::chrono::milliseconds,
          typename timep_t = std::chrono::time_point<clock_t, duration_t> >
std::tuple<timep_t, duration_t> time_since(timep_t const& start) {
    auto now = clock_t::now();
    return {now, std::chrono::duration_cast<duration_t>(now - start)};
}
