#include "donde/feature_extract/face_pipeline.h"
#include "donde/video_process/ffmpeg_processor.h"
#include "mainwindow.h"
#include "picture_factory.h"

#include <QApplication>

using json = nlohmann::json;
using donde_toolkits::feature_extract::FacePipeline;
using donde_toolkits::video_process::FFmpegVideoFrameProcessor;
using donde_toolkits::video_process::FFmpegVideoProcessor;
using donde_toolkits::video_process::ProcessOptions;
using donde_toolkits::video_process::VideoStreamInfo;

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    FFmpegVideoProcessor p{"/tmp/Iron_Man-Trailer_HD.mp4"};
    PictureFactory factory{p, 20};
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

    ProcessOptions opts{
        .warm_up_frames = 0,
        .skip_frames = 1,
        .decode_fps = 60,
        .loop_forever = true,
    };
    VideoStreamInfo info = p.Process(opts);
    w.SetVideoOpenSuccess(info.open_success);
    w.SetVideoTotalFrames(info.nb_frames);
    w.SetVideoDurationSeconds(info.duration_s);

    return a.exec();
}
