#pragma once

#include <SDL_opengl.h>
#include <string>

bool LoadImageFromFile(const std::string& filename, GLuint& out_texture, int& out_width, int& out_height) {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename.c_str(), &image_width, &image_height, NULL, 4);
    if (image_data == nullptr) {
        return false;
    }

    GLuint image_texture;
    glGenTextures(GL_TEXTURE_2D, &image_texture);
    return true;
}
