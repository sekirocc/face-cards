#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>

namespace human_card {
struct CardImage {
    CardImage(){};
    CardImage(const cv::Mat& big, const cv::Mat& small, const cv::Rect& rect)
        : big_frame(big), small_face(small), face_rect(rect){};
    cv::Mat big_frame;
    cv::Mat small_face;
    cv::Rect face_rect;
};

struct PeopleCard {
    std::string name;
    std::vector<CardImage> images;
};
} // namespace human_card
