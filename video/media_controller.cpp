#include "media_controller.h"

#include <donde/video_process/ffmpeg_processor.h>

using donde_toolkits::video_process::FFmpegVideoProcessor;
using donde_toolkits::video_process::VideoStreamInfo;

MediaController::MediaController(FFmpegVideoProcessor& processor) : video_processor{processor} {};

VideoStreamInfo MediaController::Reload(const std::string& filename) {
    return video_processor.OpenVideoContext(filename);
}

bool MediaController::Start() {
    video_processor.Process({
        .warm_up_frames = 0,
        .skip_frames = 1,
        .decode_fps = 30,
        .loop_forever = true,
    });

    state.playing_status = PlayingStatus::PLAYING;
    return true;
}

bool MediaController::Pause() {
    video_processor.Pause();

    state.playing_status = PlayingStatus::PAUSED;
    return true;
}

bool MediaController::Resume() {
    video_processor.Resume();

    state.playing_status = PlayingStatus::PLAYING;
    return true;
}

bool MediaController::Stop() {
    video_processor.Stop();

    state.playing_status = PlayingStatus::STOPPED;
    return true;
}

bool MediaController::Forward(int step) { return true; }

bool MediaController::Backward(int step) { return true; }

bool MediaController::Seek(int position) { return true; }
