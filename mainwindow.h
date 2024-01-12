#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "people_card.h"
#include "picture_generator.h"
#include "play_controller.h"

#include <QMainWindow>
#include <QtMultimediaWidgets/QVideoWidget>
#include <donde/feature_extract/face_pipeline.h>
#include <opencv2/core/mat.hpp>
#include <qabstractspinbox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qboxlayout.h>

using donde_toolkits::feature_extract::FacePipeline;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(PictureGenerator& picture_factory,
               PlayController& media_controller,
               FacePipeline& pipeline,
               QWidget* parent = nullptr);

    ~MainWindow();

    void SetVideoOpenSuccess(bool);
    void SetVideoDurationSeconds(int64_t);
    void SetVideoTotalFrames(int64_t);

  private:
    void show_video_cover();
    void loop_video_pictures();

    void display_cv_image(const cv::Mat& mat);
    void display_arbg_image(const QImage& imageARBG);
    void display_picture(const VideoPicture& pic);

    void update_detected_people();

  private slots:
    void onStartBtnClicked();
    void onStopBtnClicked();

  private:
    Ui::MainWindow* ui;

    QVideoWidget* video_display_widget;
    QVideoSink* video_display_sink;

    QLineEdit* txt_video_filepath;
    // std::vector<human_card::PeopleCard> detected_people_cards;
    std::unordered_map<std::string, human_card::PeopleCard> detected_people_cards;
    int selected_people_card_index = -1;

    QWidget* detected_people_area;

    QPushButton* btn_stop;
    QPushButton* btn_start;

    QProgressBar* pgb_video_process;

    std::thread picture_thread;

    PictureGenerator& picture_factory;
    PlayController& media_controller;
    FacePipeline& face_pipeline;

    bool video_open_success;
    int64_t video_duration_s;
    int64_t video_total_frames;
};

#endif // MAINWINDOW_H
