#include "utils.h"
#include <SDL_opengl.h>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool LoadImageFromFile(const std::string& filename, GLuint& out_texture, int& out_width, int& out_height,
                       std::vector<uint8_t>& out_image_data) {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename.c_str(), &image_width, &image_height, NULL, 4);
    if (image_data == nullptr) {
        return false;
    }
    size_t num_bytes = image_width * image_height * 4;
    out_image_data.reserve(num_bytes);
    memcpy(out_image_data.data(), image_data, num_bytes);

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

    out_texture = image_texture;
    out_width = image_width;
    out_height = image_height;

    return true;
}

