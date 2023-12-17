#include <string>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

namespace human_card {
struct CardImage {
    cv::Mat big_frame;
    cv::Mat small_face;
    cv::Rect face_rect;
};

struct PeopleCard {
    std::string name;
    std::vector<CardImage> images;
};
}  // namespace human_card
