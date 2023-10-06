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

    txt_video_filepath = ui->txt_video_filepath;
    btn_stop = ui->btn_stop;
    btn_start = ui->btn_start;
    pgb_video_process = ui->pgb_video_process;

    // ui->startButton;
    // ui->resumeButton;

    picture_thread = std::thread([&] { display_picture(); });

    connect(btn_start, &QPushButton::clicked, this, &MainWindow::onStartBtnClicked);
    connect(btn_stop, &QPushButton::clicked, this, &MainWindow::onStopBtnClicked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::display_picture() {
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

        int progress = pic.id_ * 100 / video_total_frames;
        pgb_video_process->setValue(progress);

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

void MainWindow::onStartBtnClicked() {
    auto currentState = media_controller.CurrentState();

    switch (currentState.playing_status) {

    case PlayingStatus::NOT_STARTED:
    case PlayingStatus::FINISHED:
    case PlayingStatus::STOPPED: {
        // use brackets to avoid "crosses initialization error"

        std::string filepath = txt_video_filepath->text().toStdString();
        if (filepath.empty()) {
            std::cerr << "please input video filepath" << std::endl;
            return;
        }

        auto info = media_controller.Reload(filepath);

        SetVideoOpenSuccess(info.open_success);
        SetVideoTotalFrames(info.nb_frames);
        SetVideoDurationSeconds(info.duration_s);

        media_controller.Start();
        btn_start->setText("Pause");
        break;
    }

    case PlayingStatus::PLAYING:
        media_controller.Pause();
        btn_start->setText("Resume");
        break;

    case PlayingStatus::PAUSED:
        media_controller.Resume();
        btn_start->setText("Pause");
        break;
    }
}

void MainWindow::onStopBtnClicked() {
    media_controller.Stop();
    btn_start->setText("Start");
}

void MainWindow::SetVideoOpenSuccess(bool succ) {
    video_open_success = succ;
    if (!video_open_success) {
        // TODO do what?
    }
};
void MainWindow::SetVideoDurationSeconds(int64_t duration) { video_duration_s = duration; };
void MainWindow::SetVideoTotalFrames(int64_t total_frames) { video_total_frames = total_frames; };
