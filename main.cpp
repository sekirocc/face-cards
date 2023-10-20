#include "donde/feature_extract/face_pipeline.h"
#include "donde/feature_extract/processor_factory.h"
#include "donde/video_process/ffmpeg_processor.h"
#include "picture_factory.h"

#include "window.h"

using json = nlohmann::json;
using donde_toolkits::feature_extract::FacePipeline;
using donde_toolkits::video_process::FFmpegVideoFrameProcessor;
using donde_toolkits::video_process::FFmpegVideoProcessor;

int main(int argc, char* argv[]) {
    /*
    QApplication a(argc, argv);

    json conf = R"(
    {
        "detector": {
          "concurrent": 1,
          "device_id": "CPU",
          "model": "./contrib/models/face-detection-adas-0001.xml",
          "warmup": false
        }
    }
)"_json;

    // init face pipeline, with detector only.
    FacePipeline pipeline{conf};
    auto detector = donde_toolkits::feature_extract::ProcessorFactory::createDetector();
    pipeline.Init(detector, nullptr, nullptr, nullptr);

    FFmpegVideoProcessor p{};
    PictureFactory factory{p};
    MediaController controller{p};

    MainWindow w{factory, controller, pipeline};
    w.show();

    return a.exec();
     */

    human_card::Window window{1280, 720};
    if (!window.init()) {
        return -1;
    };
    window.run();
    window.cleanup();

    return 0;
}
