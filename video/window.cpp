#include "window.h"
#include "imgui.h"
#include "fmt/core.h"

namespace human_card {

Window::Window(int width, int height) : width_(width), height_(height) {
    detectedPeopleCards.push_back(PeopleCard{.name = "Alice", .show_card = false});
    detectedPeopleCards.push_back(PeopleCard{.name = "Bob", .show_card = false});
    detectedPeopleCards.push_back(PeopleCard{.name = "Candy", .show_card = false});
};

bool Window::init() {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_,
                              height_, window_flags);

    gl_context = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
};

void Window::run() {
    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui
        // wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main
        // application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
        // application, or clear/overwrite your copy of the keyboard data. Generally you may always
        // pass all inputs to dear imgui, and hide them from your application based on those two
        // flags.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        render();
    }
};

void Window::render() {
    ImGuiIO& io = ImGui::GetIO();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    auto* vp = ImGui::GetMainViewport();
    auto w = vp->Size[0];
    auto h = vp->Size[1];

    // 1. show ImageGround window.
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(w - 200, h));

        ImGui::Begin(
            "ImageGround", NULL,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        auto currSize = ImGui::GetWindowSize();

        ImGui::Text("Video Path: %s", videoPath.c_str());
        ImGui::ProgressBar(sinf((float)ImGui::GetTime()) * 0.5f + 0.5f, ImVec2(currSize.x, 0));
        ImGui::End();
    }

    // 2. show Toolbar window
    {
        ImGui::SetNextWindowPos(ImVec2(w - 200, 0));
        ImGui::SetNextWindowSize(ImVec2(200, h));

        ImGui::Begin(
            "Detections", NULL,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        bool global_toggle_expand = false;
        if (ImGui::Button("expand all")) {
            for (auto& peopleCard : detectedPeopleCards) peopleCard.show_card = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("collapse all")) {
            for (auto& peopleCard : detectedPeopleCards) peopleCard.show_card = false;
        }

        for (auto& peopleCard : detectedPeopleCards) {
            ImGui::SetNextItemOpen(peopleCard.show_card);
            if (ImGui::CollapsingHeader(peopleCard.name.c_str()))
                peopleCard.show_card = true;
            else
                peopleCard.show_card = false;
        }

        int i = 0;
        for (auto& peopleCard : detectedPeopleCards) {
            if (peopleCard.show_card) {
                // 3. show card window
                ImVec2 startPoint{i * 100.0f, h - 400};
                ImVec2 size{400, 400};
                // resizable/movable
                ImGui::SetNextWindowPos(startPoint, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

                ImGui::Begin(fmt::format("card {}", peopleCard.name).c_str());
                ImGui::End();
                i++;
            }
        }

        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
};

void Window::cleanup() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
};

};  // namespace human_card
