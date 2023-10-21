#include "window.h"
#include <SDL_opengl.h>
#include <libyuv/convert_argb.h>
#include "imgui.h"
#include "fmt/core.h"
#include "libyuv.h"

namespace human_card {

Window::Window(int width, int height) : initViewportWidth(width), initViewportHeight(height) {
    lastViewportWidth = initViewportWidth;
    lastViewportHeight = initViewportHeight;

    detectedPeopleCards.push_back(PeopleCard{.name = "Alice", .show_card = false});
    detectedPeopleCards.push_back(PeopleCard{.name = "Bob", .show_card = false});
    detectedPeopleCards.push_back(PeopleCard{.name = "Candy", .show_card = false});
};

bool Window::init(PictureFactory* factory, PlayController* controller) {
    this->factory = factory;
    this->controller = controller;
    return this->init_gui();
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
    window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              initViewportWidth, initViewportHeight, window_flags);

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

    glGenTextures(1, &frameTexture);

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
        }

        VideoPicture* pic = factory->try_next();
        if (pic != nullptr) {
            frameWidth = pic->Width();
            frameHeight = pic->Height();
            cv::Mat mat(frameWidth, frameHeight, CV_8UC3, pic->frame->data[0], pic->frame->linesize[0]);

            // draw frame id
            std::string picId = fmt::format("{}", pic->id_);
            cv::Point posi{100, 100};
            int face = cv::FONT_HERSHEY_PLAIN;
            double scale = 2;
            cv::Scalar color{255, 0, 0};  // blue, BGR
            cv::putText(mat, picId, posi, face, scale, color, 2);

            // // detect face
            // std::shared_ptr<donde_toolkits::DetectResult> detect_result = face_pipeline.Detect(mat);
            // for (auto& face : detect_result->faces) {
            //     if (face.confidence > 0.8) {
            //         cv::Rect box = face.box;
            //         cv::rectangle(mat, box.tl(), box.br(), cv::Scalar(0, 255, 0));
            //     }
            // }

            // int progress = pic->id_ * 100 / video_total_frames;
            // pgb_video_process->setValue(progress);

            glBindTexture(GL_TEXTURE_2D, frameTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0, GL_BGR, GL_UNSIGNED_INT, mat.data);
        }

        // render gui
        render();
    }
};

void Window::display_cv_image(const cv::Mat& mat) {}

bool Window::relayout() {
    auto* vp = ImGui::GetMainViewport();
    currViewportWidth = vp->Size[0];
    currViewportHeight = vp->Size[1];
    if (currViewportWidth == lastViewportWidth && currViewportHeight == lastViewportHeight) {
        return false;
    }

    currMainWindowWidth = currViewportWidth - 200;
    currMainWindowHeight = currViewportHeight;

    currSideWindowWidth = 200;
    currSideWindowHeight = currViewportHeight;

    lastViewportWidth = currViewportWidth;
    lastViewportHeight = currViewportHeight;

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
        ImGui::SetNextWindowSize(ImVec2(currMainWindowWidth, currMainWindowHeight));

        ImGui::Begin(
            "ImageGround", NULL,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        auto currSize = ImGui::GetWindowSize();

        ImGui::Text("Video Path: %s", videoPath.c_str());
        ImGui::ProgressBar(sinf((float)ImGui::GetTime()) * 0.5f + 0.5f, ImVec2(currSize.x, 0));

        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        // ImGui::Image();

        ImGui::End();
    }

    // 2. show Toolbar window
    {
        ImGui::SetNextWindowPos(ImVec2(currMainWindowWidth, 0));
        ImGui::SetNextWindowSize(ImVec2(currSideWindowWidth, currSideWindowHeight));

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
                ImVec2 startPoint{i * 100.0f, static_cast<float>((currViewportHeight - 400))};
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