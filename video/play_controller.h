#pragma once

#include "picture_generator.h"

#include <iostream>

using donde_toolkits::video_process::VideoStreamInfo;

class VideoContext;

enum class PlayingStatus {
    NOT_STARTED,
    PLAYING,
    PAUSED,
    STOPPED,
    FINISHED,
};

struct MediaPlayState {
    PlayingStatus playing_status;

    size_t window_width;
    size_t window_height;

    float progress;
    int volume;

    std::string media_filepath;
    size_t media_filesize;
    size_t media_duration;
};

class PlayController {
   public:
    PlayController(FFmpegVideoProcessor& video_ctx);

    const MediaPlayState& CurrentState() const { return state; };

    VideoStreamInfo Reload(const std::string& filename);

    bool Start();
    bool Pause();
    bool Resume();
    bool Stop();

    bool Forward(int step);
    bool Backward(int step);
    bool Seek(int position);

   private:
    MediaPlayState state;
    FFmpegVideoProcessor& video_processor;

    VideoStreamInfo info;
    long curr_frame_id = 0;
};
