#include "engine.h"

#include "utils.h"

namespace peoplecards {
Engine::Engine(PictureGenerator& picture_factory, PlayController& media_controller, FacePipeline& pipeline)
    : media_controller(media_controller), picture_factory(picture_factory), face_pipeline(pipeline) {

    for (int i = 0; i < 1; i++) {
        un_classified_card_images.push_back(peoplecards::CardImage());
    }
};

void Engine::Start() {
    is_running = true;
    picture_thread = std::thread([&] { loop_video_pictures(); });
};

void Engine::Stop() { is_running = false; };

CardImageList& Engine::UnClassifiedCardImages() { return un_classified_card_images; };

PeopleCards& Engine::ClassifiedCardImages() { return detected_people_cards; };

void Engine::loop_video_pictures() {
    while (is_running) {
        VideoPicture* pic = picture_factory.next_frame();
        if (pic == nullptr) {
            // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        auto should_sample = pic->Id() % 30 == 0;

        auto frame_width = pic->Width();
        auto frame_height = pic->Height();
        cv::Mat mat(frame_height, frame_width, CV_8UC3, pic->frame->data[0], pic->frame->linesize[0]);

        // draw frame id
        std::string pic_id = fmt::format("{}", pic->id_);
        cv::Point posi{100, 100};
        int face = cv::FONT_HERSHEY_PLAIN;
        double scale = 2;
        cv::Scalar color{255, 0, 0}; // blue, BGR
        cv::putText(mat, pic_id, posi, face, scale, color, 2);

        auto t1 = std::chrono::steady_clock::now();
        // detect face
        std::shared_ptr<donde_toolkits::DetectResult> detect_result = face_pipeline.Detect(mat);
        for (auto& face : detect_result->faces) {
            if (face.confidence > 0.8) {
                cv::Rect box = face.box;
                // FIXME: save to first detected cards?
                auto cloned = mat.clone();
                auto size = cloned.size();
                if (box.x <= 0 || box.x >= size.width) {
                    continue;
                }
                if (box.y <= 0 || box.y >= size.height) {
                    continue;
                }
                if (box.x + box.width > size.width) {
                    std::cerr << "box width overflow!" << std::endl;
                    box.width = size.width - box.x;
                }
                if (box.y + box.height > size.height) {
                    std::cerr << "box height overflow!" << std::endl;
                    box.height = size.height - box.y;
                }

                expandBox(box, 0.2, cv::Rect(0, 0, size.width, size.height));

                // draw on original mat
                cv::rectangle(mat, box.tl(), box.br(), cv::Scalar(0, 255, 0));

                if (should_sample) {
                    cv::cvtColor(cloned, cloned, cv::COLOR_BGR2RGB);
                    un_classified_card_images.emplace_back(cloned, cloned(box), box);
                }
            }
        }

        if (should_sample) {
            auto [t2, used_ms] = now_time_since(t1);
            printf("face pipeline detect use time: %lld ms, pic_id: %ld, detected faces: %ld\n",
                   used_ms.count(),
                   pic->id_,
                   detect_result->faces.size());
        }

        // image_receiver(mat);
        emit displayCvMat(mat);

        if (should_sample) {
            emit updateUI();
        }
    }
}

} // namespace peoplecards
