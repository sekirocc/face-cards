#include <QVideoFrameFormat>
#include <QVideoFrame>
#include <QVideoSink>
#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(
        PictureFactory &pictureFactory, MediaController &mediaController,
        QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), media_controller(mediaController), picture_factory(pictureFactory) {

    ui->setupUi(this);

    video_widget = ui->video_display;
    video_sink = video_widget->videoSink();
    // ui->startButton;
    // ui->resumeButton;


    picture_thread = std::thread(&MainWindow::consume_picture, this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::consume_picture()
{
    while (true)
    {
        VideoPicture &pic = picture_factory.next();
        display_picture(pic);
    }
}

void MainWindow::display_picture(const VideoPicture &pic)
{
    QSize size(pic.Width(), pic.Height());
    QVideoFrameFormat fmt(size, QVideoFrameFormat::Format_YUV420P);
    QVideoFrame frame(fmt);
    frame.map(QVideoFrame::ReadWrite);

    int y_size = pic.Width() * pic.Height();
    int uv_size = y_size / 4;

    uint8_t *pY = pic.frame->data[0];
    uint8_t *pU = pic.frame->data[1];
    uint8_t *pV = pic.frame->data[2];

    // copy y plane
    memcpy(frame.bits(0), pY, y_size);
    // copy u plane
    memcpy(frame.bits(1), pU, uv_size);
    // copy v plane
    memcpy(frame.bits(1) + uv_size, pV, uv_size);

    frame.unmap();

    video_sink->setVideoFrame(frame);
}
