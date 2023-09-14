#pragma once

#include <condition_variable>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <memory>
#include <string>
#include <thread>

#include "channel.h"
#include "media_controller.h"

// borrow the frame pointer. donot own it. donot free it.
// using FrameProcessor = bool (*)(const AVFrame *f);
using FrameProcessor = std::function<bool(const AVFrame *f)>;

class VideoContext
{
  public:
    VideoContext(const std::string &filepath);

    const MediaController &GetController() const;

    bool Process();
    bool Register(const FrameProcessor &p);

    bool Pause();
    bool Resume();

    bool Stop();

    SwsContext *GetSwsContext() const
    {
        return sws_context_;
    };

    ~VideoContext();

  private:
    bool open_context();

    void demux_video_packet_();
    void decode_video_frame_();
    void process_video_frame_();

  private:
    std::string filepath;

    AVFormatContext *format_context_ = nullptr;
    AVCodecContext *codec_context_ = nullptr;
    SwsContext *sws_context_ = nullptr;
    int video_stream_index_ = -1;

    size_t video_width_ = 0;
    size_t video_height_ = 0;

    MediaController *controller = nullptr;
    bool quit_ = false;
    bool pause_ = false;

    std::mutex demux_mu_;
    std::condition_variable demux_cv_;

    std::thread demux_thread_;
    std::thread decode_thread_;
    std::thread process_thread_;

    size_t frame_count = 0;
    FrameProcessor frame_processor;

    // used to send demuxed packet
    Channel<AVPacket *> packet_ch_;
    // used to send decoded frame
    Channel<AVFrame *> frame_ch_;
};
