#pragma once

#include <SDL_opengl.h>
#include <string>

bool LoadImageFromFile(const std::string& filename, GLuint& out_texture, int& out_width, int& out_height,
                       std::vector<uint8_t>& out_image_data);
