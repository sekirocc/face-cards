#include "play_controller.h"
#include "fmt/format.h"

#include <donde/video_process/ffmpeg_processor.h>

using donde_toolkits::video_process::FFmpegVideoFrame;
using donde_toolkits::video_process::FFmpegVideoProcessor;
using donde_toolkits::video_process::VideoStreamInfo;

PlayController::PlayController(FFmpegVideoProcessor& processor) : video_processor{processor} {};

VideoStreamInfo PlayController::Reload(const std::string& filename) {
    info = video_processor.OpenVideoContext(filename);
    std::cout << fmt::format(
        "open video info: nb_frames {}, avg_frame_rate: {}, duration_seconds: {}, total_frames: {}, width: {}, height: {}",
        info.nb_frames,
        info.avg_frame_rate,
        info.duration_seconds,
        info.avg_frame_rate * info.duration_seconds,
        info.width,
        info.height);
    return info;
}

void PlayController::Start() {
    video_processor.Process({
        .warm_up_frames = 0,
        .skip_frames = 1,
        .decode_fps = 30,
        .loop_forever = true,
    });
    video_processor.Register([&](const FFmpegVideoFrame* frame) -> bool {
        curr_frame_id = frame->getFrameId();
        state.progress = static_cast<float>(curr_frame_id) / (info.duration_seconds * info.avg_frame_rate);
        return true;
    });
    state.playing_status = PlayingStatus::PLAYING;
}

void PlayController::Pause() {
    video_processor.Pause();
    state.playing_status = PlayingStatus::PAUSED;
}

void PlayController::Resume() {
    video_processor.Resume();
    state.playing_status = PlayingStatus::PLAYING;
}

void PlayController::TogglePlay() {
    if (state.playing_status == PlayingStatus::PLAYING)
        Pause();
    else
        Resume();
}

bool PlayController::IsPlaying() { return state.playing_status == PlayingStatus::PLAYING; }

void PlayController::Stop() {
    video_processor.Stop();
    state.playing_status = PlayingStatus::STOPPED;
}

void PlayController::Forward(int step) {}

void PlayController::Backward(int step) {}

void PlayController::Seek(int position) {}
