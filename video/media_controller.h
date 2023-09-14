#pragma once

#include <iostream>

class VideoContext;

struct MediaPlayState
{
    bool is_playing;

    size_t window_width;
    size_t window_height;

    size_t progress;
    int volume;

    std::string media_filepath;
    size_t media_filesize;
    size_t media_duration;
};

class MediaController
{

  public:
    MediaController(VideoContext &video_ctx);

    const MediaPlayState &CurrentState() const;

    bool Reload(std::string filename);
    bool Pause();
    bool Resume();

    bool Terminate();

    bool Forward(int step);
    bool Backward(int step);
    bool Seek(int position);

    MediaPlayState state;

    VideoContext &video_ctx;
};
