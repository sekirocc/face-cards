#include "donde/feature_extract/face_pipeline.h"
#include "donde/video_process/ffmpeg_processor.h"
#include "mainwindow.h"
#include "picture_factory.h"
#include "video_context.h"

#include <QApplication>

using json = nlohmann::json;
using donde_toolkits::feature_extract::FacePipeline;
using donde_toolkits::video_process::FFmpegVideoFrameProcessor;
using donde_toolkits::video_process::FFmpegVideoProcessor;

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    FFmpegVideoProcessor p{"/tmp/Iron_Man-Trailer_HD.mp4"};
    PictureFactory factory{p, 20};
    p.Process();

    VideoContext video_ctx{"/tmp/Iron_Man-Trailer_HD.mp4"};
    MediaController controller{p};

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
