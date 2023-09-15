#include "mainwindow.h"
#include "video_context.h"
#include "picture_factory.h"
#include "donde/feature_extract/face_pipeline.h"

#include <QApplication>

using json = nlohmann::json;
using donde_toolkits::feature_extract::FacePipeline;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoContext video_ctx{"/tmp/Iron_Man-Trailer_HD.mp4"};
    MediaController controller{video_ctx};
    PictureFactory factory{video_ctx};

    json conf = R"(
    {
        "detector": {
          "concurrent": 1,
          "device_id": "CPU",
          "model": "./contrib/models/face-detection-adas-0001.xml",
          "warmup": false
        },
        "landmarks": {
          "concurrent": 1,
          "device_id": "CPU",
          "model": "./contrib/models/facial-landmarks-35-adas-0002.xml",
          "warmup": false
        },
        "aligner": {
          "concurrent": 1,
          "device_id": "CPU",
          "warmup": false
        },
        "feature": {
          "concurrent": 1,
          "device_id": "CPU",
          "model": "./contrib/models/Sphereface.xml",
          "warmup": false
        }
    }
)"_json;

    FacePipeline pipeline{conf};

    MainWindow w{factory, controller};
    w.show();

    video_ctx.Process();
    return a.exec();
}
