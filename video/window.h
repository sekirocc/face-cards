// clang-format: off
#include "imgui.h"
#include <SDL.h>
#include <donde/feature_extract/face_pipeline.h>
#include <stdio.h>
#include <SDL_opengl.h>
#include <string>
#include <unordered_map>
// clang-format: on

#include "play_controller.h"
#include "picture_generator.h"
#include "people_card.h"

using donde_toolkits::feature_extract::IFacePipeline;

namespace human_card {
class Window {
   public:
    Window(int width, int height);
    bool init(PictureGenerator* factory, PlayController* controller, IFacePipeline* face_pipeline);
    void run();
    void cleanup();

   private:
    bool init_gui();
    void render();
    bool relayout();

    void prepare_frame_texture(VideoPicture* pic);

   private:
    int init_viewport_width;
    int init_viewport_height;
    SDL_Window* window;

    int last_viewport_width;
    int last_viewport_height;

    int curr_viewport_width;
    int curr_viewport_height;

    int curr_main_window_width;
    int curr_main_window_height;
    int curr_side_window_width;
    int curr_side_window_height;

    SDL_GLContext gl_context;

    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::string video_path = "";
    std::vector<PeopleCard> detected_people_cards;

    PictureGenerator* pic_gen = nullptr;
    PlayController* controller = nullptr;
    IFacePipeline* face_pipeline = nullptr;

    std::vector<uint8_t> cover_frame_data;
    GLuint cover_frame_texture;
    int cover_frame_width;
    int cover_frame_height;

    GLuint frame_texture;
    int frame_width;
    int frame_height;
    GLenum frame_format = GL_RGB;
};
}  // namespace human_card
