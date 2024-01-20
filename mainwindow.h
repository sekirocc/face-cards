#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Section.h"
#include "engine.h"
#include "people_card.h"
#include "picture_generator.h"
#include "play_controller.h"

#include <QMainWindow>
#include <QtMultimediaWidgets/QVideoWidget>
#include <donde/feature_extract/face_pipeline.h>
#include <opencv2/core/mat.hpp>
#include <qabstractspinbox.h>
#include <qboxlayout.h>
#include <qprogressbar.h>
#include <qpushbutton.h>

using donde_toolkits::feature_extract::FacePipeline;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

using CardImageList = std::vector<human_card::CardImage>;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(PictureGenerator& picture_factory,
               PlayController& media_controller,
               FacePipeline& pipeline,
               QSize windowSize,
               QRect windowRect,
               QWidget* parent = nullptr);

    ~MainWindow();

    void SetVideoOpenSuccess(bool);
    void SetVideoDurationSeconds(int64_t);
    void SetVideoTotalFrames(int64_t);

  private:
    void show_video_cover();

    bool display_cv_image(const cv::Mat& mat);
    void display_arbg_image(const QImage& imageARBG);
    void display_picture(const VideoPicture& pic);

    void init_detected_card_images_area();

    void update_section(const std::string& name, const CardImageList& images);
    ui::Section* create_section(const std::string& name, const CardImageList& images);
    void update_sections_if_need();

  private slots:
    void onStartBtnClicked();
    void onStopBtnClicked();

  signals:
    void updateUI();

  private:
    Ui::MainWindow* ui;

    QVideoWidget* video_display_widget;
    QVideoSink* video_display_sink;

    QLineEdit* txt_video_filepath;

    QWidget* detected_people_area;

    QPushButton* btn_stop;
    QPushButton* btn_start;

    QProgressBar* pgb_video_process;

    int selected_people_card_index = -1;

    PlayController& media_controller;

    std::shared_ptr<peoplecards::Engine> engine;

    bool video_open_success;
    int64_t video_duration_s;
    int64_t video_total_frames;
};

#endif // MAINWINDOW_H
