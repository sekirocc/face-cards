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

using CardImageList = std::vector<peoplecards::CardImage>;
using PeopleCards = std::unordered_map<std::string, peoplecards::PeopleCard>;

class Engine : public QObject {

    Q_OBJECT

  public:
    Engine(PictureGenerator& picture_factory, PlayController& media_controller, FacePipeline& pipeline);
    void Start();
    void Stop();

    CardImageList& UnClassifiedCardImages();
    PeopleCards& ClassifiedCardImages();

  signals:
    void updateUI();
    void displayCvMat(const cv::Mat& mat);

  private:
    void loop_video_pictures();

    bool is_running;
    std::thread picture_thread;

    // std::vector<peoplecards::PeopleCard> detected_people_cards;
    PeopleCards detected_people_cards;
    CardImageList un_classified_card_images;

    PictureGenerator& picture_factory;
    PlayController& media_controller;
    FacePipeline& face_pipeline;
};

} // namespace peoplecards

#endif // ENGINE_H
