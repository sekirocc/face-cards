#include <string>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

namespace human_card {
struct CardImage {
    cv::Mat frame;
};

struct PeopleCard {
    std::string name;
    std::vector<CardImage> images;
    bool show_card;
};
}  // namespace human_card
