#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "media_controller.h"
#include "picture_factory.h"

#include <QMainWindow>
#include <QtMultimediaWidgets/QVideoWidget>

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

  private:
    void consume_picture();

    void display_picture(const VideoPicture& pic);

  private:
    Ui::MainWindow* ui;

    QVideoWidget* video_widget;
    QVideoSink* video_sink;

    std::thread picture_thread;
    PictureFactory& picture_factory;
    MediaController& media_controller;
};

#endif // MAINWINDOW_H
