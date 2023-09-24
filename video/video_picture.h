#pragma once

#include <memory>
#include <string>
#include <thread>

#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

struct VideoPicture
{
    VideoPicture(): frame(nullptr){};

    VideoPicture(const VideoPicture &rhs) = delete;
    VideoPicture(VideoPicture &&rhs) = delete;
    VideoPicture operator=(const VideoPicture &rhs) = delete;
    VideoPicture operator=(VideoPicture &&rhs) = delete;

    ~VideoPicture()
    {
        if (frame)
        {
            av_frame_free(&frame);
        }
    };

    bool HasFrame() const
    {
        return frame != nullptr;
    }
    int Height() const
    {
        return height_;
    }
    int Width() const
    {
        return width_;
    }

    bool CreateFrame(int width, int height, AVPixelFormat pix_fmt)
    {
        width_ = width;
        height_ = height;
        pix_fmt_ = pix_fmt;

        if (frame)
        {
            av_frame_free(&frame);
        }

        // alloc a new frame to hold the scaled data.(through data buffer pointer)
        frame = av_frame_alloc();
        if (!frame)
        {
            std::cerr << "cannot alloc frame" << std::endl;
            return false;
        }

        // alloc data buffer pointer. this area will hold acture image data.
        data_len = av_image_get_buffer_size(pix_fmt, width, height, 32) * sizeof(uint8_t);
        data = (uint8_t *)av_malloc(data_len);
        if (!data)
        {
            return false;
        }

        // fill in frame buffer pointers to specific position in data.
        int ret = av_image_fill_arrays(frame->data, frame->linesize, data, pix_fmt, width, height, 32);
        if (ret < 0)
        {
            std::cerr << "cannot fill image array" << std::endl;
            return false;
        }

        return true;
    }

    // bool Scale(SwsContext *sws_ctx, const AVFrame *originFrame)
    // {
    //     frame->pts = originFrame->pts;
    //     frame->key_frame = originFrame->key_frame;
    //     frame->coded_picture_number = originFrame->coded_picture_number;
    //     frame->display_picture_number = originFrame->display_picture_number;
    //     frame->width = originFrame->width;
    //     frame->height = originFrame->height;

    //     sws_scale(sws_ctx, originFrame->data, originFrame->linesize, 0, height_, frame->data, frame->linesize);

    //     return true;
    // }

    size_t width_;
    size_t height_;
    AVPixelFormat pix_fmt_;

    int data_len;
    uint8_t *data;

    AVFrame *frame;
};

using VideoPicturePtr = std::shared_ptr<VideoPicture>;
