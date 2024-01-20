#ifndef ENGINE_H
#define ENGINE_H

#include "people_card.h"
#include "picture_generator.h"
#include "play_controller.h"

#include <donde/feature_extract/face_pipeline.h>
#include <functional>
#include <opencv2/core/mat.hpp>
#include <qobject.h>

using donde_toolkits::feature_extract::FacePipeline;

namespace peoplecards {

using ImageReceiverFunc = std::function<bool(const cv::Mat& mat)>;

using CardImageList = std::vector<human_card::CardImage>;

class Engine : public QObject {

    Q_OBJECT

  public:
    Engine(PictureGenerator& picture_factory,
           PlayController& media_controller,
           FacePipeline& pipeline);
    void Start(ImageReceiverFunc image_receiver);
    void Stop();

    CardImageList& UnClassifiedCardImages();
    std::unordered_map<std::string, human_card::PeopleCard>& ClassifiedCardImages();

  signals:
    void updateUI();

  private:
    void loop_video_pictures();

    bool is_running;
    std::thread picture_thread;

    // std::vector<human_card::PeopleCard> detected_people_cards;
    std::unordered_map<std::string, human_card::PeopleCard> detected_people_cards;
    CardImageList un_classified_card_images;

    PictureGenerator& picture_factory;
    PlayController& media_controller;
    FacePipeline& face_pipeline;
    ImageReceiverFunc image_receiver;
};

} // namespace peoplecards

#endif // ENGINE_H
