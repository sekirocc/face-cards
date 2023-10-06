#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "media_controller.h"
#include "picture_factory.h"

#include <QMainWindow>
#include <QtMultimediaWidgets/QVideoWidget>
#include <opencv2/core/mat.hpp>
#include <qprogressbar.h>
#include <qpushbutton.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(PictureFactory& picture_factory, MediaController& media_controller,
               QWidget* parent = nullptr);

    ~MainWindow();

    void SetVideoOpenSuccess(bool);
    void SetVideoDurationSeconds(int64_t);
    void SetVideoTotalFrames(int64_t);

  private:
    void display_picture();

    void display_cv_image(const cv::Mat& mat);
    void display_picture(const VideoPicture& pic);

  private:
    Ui::MainWindow* ui;

    QVideoWidget* video_display_widget;
    QVideoSink* video_display_sink;

    QPushButton* btn_pause_resume;
    QPushButton* btn_start;

    QProgressBar* pgb_video_process;

    std::thread picture_thread;
    PictureFactory& picture_factory;
    MediaController& media_controller;

    bool video_open_success;
    int64_t video_duration_s;
    int64_t video_total_frames;
};

#endif // MAINWINDOW_H
