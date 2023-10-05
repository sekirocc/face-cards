#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "libyuv.h"

#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>
#include <fmt/format.h>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(PictureFactory& pictureFactory,
                       MediaController& mediaController,
                       QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      media_controller(mediaController),
      picture_factory(pictureFactory) {

    ui->setupUi(this);

    video_display_widget = ui->video_display;
    video_display_sink = video_display_widget->videoSink();

    pause_resume_btn = ui->resumeButton;
    start_btn = ui->startButton;
    video_process_progressbar = ui->video_process_progressbar;
    // ui->startButton;
    // ui->resumeButton;

    picture_thread = std::thread(&MainWindow::consume_picture, this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::consume_picture() {
    while (true) {
        VideoPicture& pic = picture_factory.next();
        cv::Mat mat(pic.Height(), pic.Width(), CV_8UC3, pic.frame->data[0], pic.frame->linesize[0]);

        // draw frame id
        std::string picId = fmt::format("{}", pic.id_);
        cv::Point posi{100, 100};
        int face = cv::FONT_HERSHEY_PLAIN;
        double scale = 2;
        cv::Scalar color{255, 0, 0}; // blue, BGR
        cv::putText(mat, picId, posi, face, scale, color, 2);

        display_cv_image(mat);
    }
}

void MainWindow::display_cv_image(const cv::Mat& mat) {
    QImage imgBGR((uchar*)mat.data, mat.cols, mat.rows, QImage::Format_BGR888);
    QImage imageARBG = imgBGR.convertToFormat(QImage::Format_ARGB32);

    auto fmt = QVideoFrameFormat(imageARBG.size(), QVideoFrameFormat::Format_YUV420P);
    QVideoFrame frame(fmt);
    if (!frame.map(QVideoFrame::ReadWrite)) {
        std::cout << "cann't map frame." << std::endl;
        return;
    }
    int width = imageARBG.width();
    int height = imageARBG.height();

    libyuv::ARGBToI420((const uint8_t*)imageARBG.bits(),
                       imageARBG.bytesPerLine(),
                       frame.bits(0),
                       frame.bytesPerLine(0),
                       frame.bits(1),
                       frame.bytesPerLine(1),
                       frame.bits(2),
                       frame.bytesPerLine(2),
                       width,
                       height);
    frame.unmap();
    video_display_sink->setVideoFrame(frame);
}

// deprecated.
void MainWindow::display_picture(const VideoPicture& pic) {
    QSize size(pic.Width(), pic.Height());
    QVideoFrame frame;
    if (pic.pix_fmt_ == AV_PIX_FMT_YUV420P) {
        QVideoFrameFormat fmt(size, QVideoFrameFormat::Format_YUV420P);
        frame = QVideoFrame(fmt);
        frame.map(QVideoFrame::ReadWrite);

        int y_size = pic.Width() * pic.Height();
        int uv_size = y_size / 4;

        uint8_t* pY = pic.frame->data[0];
        uint8_t* pU = pic.frame->data[1];
        uint8_t* pV = pic.frame->data[2];

        // copy y plane
        memcpy(frame.bits(0), pY, y_size);
        // copy u plane
        memcpy(frame.bits(1), pU, uv_size);
        // copy v plane
        memcpy(frame.bits(1) + uv_size, pV, uv_size);

        frame.unmap();
    } else if (pic.pix_fmt_ == AV_PIX_FMT_BGR24) {
        QVideoFrameFormat fmt(size, QVideoFrameFormat::Format_BGRA8888);
        frame = QVideoFrame(fmt);
    }

    video_display_sink->setVideoFrame(frame);
}
