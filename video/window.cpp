#include "window.h"
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_opengl.h>
#include <libyuv/convert_argb.h>
#include <chrono>
#include <thread>
#include "imgui.h"
#include "fmt/core.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "utils.h"

// #include "libyuv.h"

namespace human_card {

Window::Window(int width, int height) : init_viewport_width(width), init_viewport_height(height) {
    last_viewport_width = init_viewport_width;
    last_viewport_height = init_viewport_height;

    detected_people_cards.push_back(PeopleCard{.name = "Alice", .show_card = false});
    detected_people_cards.push_back(PeopleCard{.name = "Bob", .show_card = false});
    detected_people_cards.push_back(PeopleCard{.name = "Candy", .show_card = false});
};

bool Window::init(PictureFactory* factory, PlayController* controller) {
    this->factory = factory;
    this->controller = controller;

    auto ret = this->init_gui();
    if (!ret) {
        return false;
    }

    auto loaded = LoadImageFromFile("/Users/bytedance/work/code/cpp/human_card/resources/images/video_cover.png",
                                    cover_frame_texture,
                                    cover_frame_width,
                                    cover_frame_height,
                                    cover_frame_data);

    if (!loaded) {
        std::cerr << "Error: cann't load default background image" << std::endl;
        return false;
    }
    std::cout << "loaded default image: " << cover_frame_width << " x " << cover_frame_height << std::endl;

    auto info = this->controller->Reload("/tmp/Iron_Man-Trailer_HD.mp4");
    std::cout << "video info: nb_frames: " << info.nb_frames << std::endl;
    this->controller->Start();

    return true;
};

bool Window::init_gui() {
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
    window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              init_viewport_width,
                              init_viewport_height,
                              window_flags);

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

    glGenTextures(1, &frame_texture);

    return true;
};

void Window::run() {
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) controller->TogglePlay();
        }

        VideoPicture* pic = factory->next();
        if (pic != nullptr) {
            frame_width = pic->Width();
            frame_height = pic->Height();
            cv::Mat mat(frame_width, frame_height, CV_8UC3, pic->frame->data[0], pic->frame->linesize[0]);

            // draw frame id
            std::string pic_id = fmt::format("{}", pic->id_);
            cv::Point posi{100, 100};
            int face = cv::FONT_HERSHEY_PLAIN;
            double scale = 2;
            cv::Scalar color{255, 0, 0};  // blue, BGR
            cv::putText(mat, pic_id, posi, face, scale, color, 2);

            // // detect face
            // std::shared_ptr<donde_toolkits::DetectResult> detect_result =
            // face_pipeline.Detect(mat); for (auto& face :
            // detect_result->faces) {
            //     if (face.confidence > 0.8) {
            //         cv::Rect box = face.box;
            //         cv::rectangle(mat, box.tl(), box.br(), cv::Scalar(0, 255,
            //         0));
            //     }
            // }

            // int progress = pic->id_ * 100 / video_total_frames;
            // pgb_video_process->setValue(progress);

            glBindTexture(GL_TEXTURE_2D, frame_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_BGR, GL_UNSIGNED_BYTE, mat.data);
        }

        // render gui
        render();
    }
};

void Window::display_cv_image(const cv::Mat& mat) {}

bool Window::relayout() {
    auto* vp = ImGui::GetMainViewport();
    curr_viewport_width = vp->Size[0];
    curr_viewport_height = vp->Size[1];

    curr_main_window_width = curr_viewport_width - 200;
    curr_main_window_height = curr_viewport_height;

    curr_side_window_width = 200;
    curr_side_window_height = curr_viewport_height;

    last_viewport_width = curr_viewport_width;
    last_viewport_height = curr_viewport_height;

    return true;
};

void Window::render() {
    ImGuiIO& io = ImGui::GetIO();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    relayout();

    // 1. show ImageGround window.
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(curr_main_window_width, curr_main_window_height));

        ImGui::Begin(
            "ImageGround",
            NULL,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        auto window_size = ImGui::GetWindowSize();

        ImGui::Text("Video Path: %s", video_path.c_str());

        float ration_w = static_cast<float>(curr_main_window_width) / frame_width;
        float ration_h = static_cast<float>(curr_main_window_height) / frame_height;
        int display_width = curr_main_window_width;
        int display_height = curr_main_window_height;
        if (ration_w < ration_h) {
            display_width = curr_main_window_width;
            display_height = ration_w * frame_height;
        } else {
            display_height = curr_main_window_height;
            display_width = ration_h * frame_width;
        }
        ImGui::Image((void*)(intptr_t)frame_texture, ImVec2(display_width, display_height));

        const auto& play_state = controller->CurrentState();
        ImGui::ProgressBar(play_state.progress, ImVec2(window_size.x, 0));
        ImGui::End();
    }

    // 2. show Toolbar window
    {
        ImGui::SetNextWindowPos(ImVec2(curr_main_window_width, 0));
        ImGui::SetNextWindowSize(ImVec2(curr_side_window_width, curr_side_window_height));

        ImGui::Begin(
            "Detections",
            NULL,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        bool global_toggle_expand = false;
        if (ImGui::Button("expand all")) {
            for (auto& people_card : detected_people_cards) people_card.show_card = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("collapse all")) {
            for (auto& people_card : detected_people_cards) people_card.show_card = false;
        }

        for (auto& people_card : detected_people_cards) {
            ImGui::SetNextItemOpen(people_card.show_card);
            if (ImGui::CollapsingHeader(people_card.name.c_str()))
                people_card.show_card = true;
            else
                people_card.show_card = false;
        }

        int i = 0;
        for (auto& people_card : detected_people_cards) {
            if (people_card.show_card) {
                // 3. show card window
                ImVec2 point{i * 100.0f, static_cast<float>((curr_viewport_height - 400))};
                ImVec2 size{400, 400};
                // resizable/movable
                ImGui::SetNextWindowPos(point, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

                ImGui::Begin(fmt::format("card {}", people_card.name).c_str());
                ImGui::End();
                i++;
            }
        }

        ImGui::End();
    }

    // Rendering
    ImGui::Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w,
                 clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w,
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
