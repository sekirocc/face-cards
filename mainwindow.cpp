#include "mainwindow.h"
#include "libyuv.h"

#include "./ui_mainwindow.h"

#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(PictureFactory& pictureFactory, MediaController& mediaController,
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
        std::cout << "get picture from factory: id: " << pic.Id() << ", width: " << pic.Width()
                  << std::endl;
        cv::Mat mat(pic.Height(), pic.Width(), CV_8UC3, pic.frame->data[0], pic.frame->linesize[0]);
        display_picture(mat);
    }
}

void MainWindow::display_picture(const cv::Mat& mat) {
    printf("mat: %d x %d \n", mat.cols, mat.rows);
    QImage img((uchar*)mat.data, mat.cols, mat.rows, QImage::Format_BGR888);

    img = img.convertToFormat(QImage::Format_ARGB32);
    auto fmt = QVideoFrameFormat(img.size(), QVideoFrameFormat::Format_YUV420P);
    QVideoFrame frame(fmt);
    if (!frame.map(QVideoFrame::ReadWrite)) {
        std::cout << "cannot map frame." << std::endl;
    }
    int width = img.width();
    int height = img.height();

    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "frame.bytesPerLine(0): " << frame.bytesPerLine(0) << std::endl;
    std::cout << "frame.bytesPerLine(1): " << frame.bytesPerLine(1) << std::endl;
    std::cout << "frame.bytesPerLine(2): " << frame.bytesPerLine(2) << std::endl;

    libyuv::ARGBToI420((const uint8_t* )img.bits(),
               img.bytesPerLine(),

               frame.bits(0),
               frame.bytesPerLine(0),
               // width * height,

               frame.bits(1),
               frame.bytesPerLine(1),
               // width / 2,

               frame.bits(2),
               frame.bytesPerLine(2),
               // width / 2,

               width,
               height);
    frame.unmap();
    video_display_sink->setVideoFrame(frame);
}

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
