// clang-format: off
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include <SDL.h>
#include <stdio.h>
#include <SDL_opengl.h>
#include <string>
#include <unordered_map>
// clang-format: on

#include "people_card.h"

namespace human_card {
class Window {
   public:
    Window(int width, int height);
    bool init();
    void run();
    void cleanup();

   private:
    void render();

   private:
    int width_;
    int height_;
    SDL_Window* window;

    SDL_GLContext gl_context;

    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::string videoPath = "";
    std::vector<PeopleCard> detectedPeopleCards;
};
}  // namespace human_card
