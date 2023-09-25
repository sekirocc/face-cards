#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>

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
        display_picture(pic);
    }
}

void MainWindow::display_picture(const VideoPicture& pic) {
    QSize size(pic.Width(), pic.Height());
    QVideoFrameFormat fmt(size, QVideoFrameFormat::Format_YUV420P);
    QVideoFrame frame(fmt);
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

    video_display_sink->setVideoFrame(frame);
}
