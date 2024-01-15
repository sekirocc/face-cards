#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Section.h"
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
    void loop_video_pictures();

    void display_cv_image(const cv::Mat& mat);
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
    // std::vector<human_card::PeopleCard> detected_people_cards;
    std::unordered_map<std::string, human_card::PeopleCard> detected_people_cards;
    std::vector<human_card::CardImage> un_classified_card_images;
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

inline void clearLayout(QLayout* layout) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->layout()) {
            clearLayout(item->layout());
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

inline void expandBox(cv::Rect& box, float factor, cv::Rect bound) {
    auto delta_x = static_cast<int>(box.width * factor);
    auto delta_y = static_cast<int>(box.height * factor);
    box.x -= delta_x;
    box.y -= delta_y;
    box.width += delta_x * 2;
    box.height += delta_y * 2;

    if (box.x < bound.x) {
        box.x = bound.x;
    }
    if (box.y < bound.y) {
        box.y = bound.y;
    }
    if (box.x + box.width > bound.x + bound.width) {
        box.width = bound.x + bound.width - box.x;
    }
    if (box.y + box.height > bound.y + bound.height) {
        box.height = bound.y + bound.height - box.y;
    }
}

#endif // MAINWINDOW_H
