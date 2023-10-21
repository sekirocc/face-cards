#include "play_controller.h"

#include <donde/video_process/ffmpeg_processor.h>

using donde_toolkits::video_process::FFmpegVideoProcessor;
using donde_toolkits::video_process::VideoStreamInfo;

PlayController::PlayController(FFmpegVideoProcessor& processor) : video_processor{processor} {};

VideoStreamInfo PlayController::Reload(const std::string& filename) {
    return video_processor.OpenVideoContext(filename);
}

bool PlayController::Start() {
    video_processor.Process({
        .warm_up_frames = 0,
        .skip_frames = 1,
        .decode_fps = 30,
        .loop_forever = true,
    });

    state.playing_status = PlayingStatus::PLAYING;
    return true;
}

bool PlayController::Pause() {
    video_processor.Pause();

    state.playing_status = PlayingStatus::PAUSED;
    return true;
}

bool PlayController::Resume() {
    video_processor.Resume();

    state.playing_status = PlayingStatus::PLAYING;
    return true;
}

bool PlayController::Stop() {
    video_processor.Stop();

    state.playing_status = PlayingStatus::STOPPED;
    return true;
}

bool PlayController::Forward(int step) { return true; }

bool PlayController::Backward(int step) { return true; }

bool PlayController::Seek(int position) { return true; }
