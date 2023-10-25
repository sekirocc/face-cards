#pragma once

#include "picture.h"
#include "msd/channel.hpp"

#include <atomic>
#include <chrono>
#include <donde/video_process/ffmpeg_processor.h>
#include <memory>
#include <string>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using donde_toolkits::video_process::FFmpegVideoFrameProcessor;
using donde_toolkits::video_process::FFmpegVideoProcessor;

class PictureGenerator {
   public:
    PictureGenerator(FFmpegVideoProcessor& video_processor, size_t pictures_len = 20)
        : video_processor_{video_processor}, pictures_len(pictures_len) {
        pictures = new VideoPicture[pictures_len];

        FFmpegVideoFrameProcessor p(std::bind(&PictureGenerator::consume, this, std::placeholders::_1));

        video_processor.Register(p);

        // for stat.
        frames_per_second = 0;
        record_time = std::chrono::steady_clock::now();
    };

    ~PictureGenerator() { delete[] pictures; }

    bool consume(const donde_toolkits::video_process::FFmpegVideoFrame* vf) {
        const AVFrame* f = (const AVFrame*)vf->getFrame();

        frames_per_second++;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - record_time).count() >= 1000) {
            std::cout << "fps: " << frames_per_second << ", frame id: " << vf->getFrameId() << std::endl;
            frames_per_second = 0;
            record_time = now;
        }

        VideoPicture& pic = pictures[write_index];
        if (!pic.HasFrame() || pic.Height() != f->height || pic.Width() != f->width) {
            pic.CreateFrame(f->width, f->height, AV_PIX_FMT_BGR24);
        }

        video_processor_.ScaleFrame(f, pic.frame);

        pic.id_ = vf->getFrameId();

        write_index++;
        if (write_index == pictures_len) {
            write_index = 0;
        }

        // std::cout << "picture display: processed frames: " << frames_processed_
        // << " write_index: " << write_index
        //           << std::endl;

        output_ch << &pic;
        return true;
    }

    VideoPicture* next() {
        if (output_ch.empty()) return nullptr;
        VideoPicture* ptr;
        output_ch >> ptr;
        return ptr;
    }

   private:
    int frames_processed_ = 0;

    const FFmpegVideoProcessor& video_processor_;

    VideoPicture* pictures;
    int pictures_len;

    msd::channel<VideoPicture*> output_ch{1};

    int write_index = 0;
    int read_index = 0;

    int frames_per_second;
    std::chrono::steady_clock::time_point record_time;
};
