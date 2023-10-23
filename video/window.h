// clang-format: off
#include "imgui.h"
#include <SDL.h>
#include <stdio.h>
#include <SDL_opengl.h>
#include <string>
#include <unordered_map>
// clang-format: on

#include "play_controller.h"
#include "picture_generator.h"
#include "people_card.h"

namespace human_card {
class Window {
   public:
    Window(int width, int height);
    bool init(PictureFactory* factory, PlayController* controller);
    void run();
    void cleanup();

   private:
    bool init_gui();
    void render();
    bool relayout();
    void display_cv_image(const cv::Mat& mat);

   private:
    int initViewportWidth;
    int initViewportHeight;
    SDL_Window* window;

    int lastViewportWidth;
    int lastViewportHeight;

    int currViewportWidth;
    int currViewportHeight;

    int currMainWindowWidth;
    int currMainWindowHeight;
    int currSideWindowWidth;
    int currSideWindowHeight;

    SDL_GLContext gl_context;

    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::string videoPath = "";
    std::vector<PeopleCard> detectedPeopleCards;

    PictureFactory* factory = nullptr;
    PlayController* controller = nullptr;

    std::vector<uint8_t> coverFrameData;
    GLuint coverFrameTexture;
    int coverFrameWidth;
    int coverFrameHeight;

    GLuint frameTexture;
    int frameWidth;
    int frameHeight;
    GLenum frameFormat = GL_RGB;
};
}  // namespace human_card
