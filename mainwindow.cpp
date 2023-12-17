#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "Section.h"
#include "libyuv.h"
#include "utils.h"

#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
using namespace std::placeholders;

MainWindow::MainWindow(PictureGenerator& picture_factory,
                       PlayController& media_controller,
                       FacePipeline& pipeline,
                       QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      media_controller(media_controller),
      picture_factory(picture_factory),
      face_pipeline(pipeline) {

    ui->setupUi(this);

    detected_people_cards.push_back(human_card::PeopleCard{.name = "Alice"});
    detected_people_cards.push_back(human_card::PeopleCard{.name = "Bob"});
    detected_people_cards.push_back(human_card::PeopleCard{.name = "Candy"});

    detected_people_area = ui->detected_people_area;
    update_detected_people();

    video_display_widget = ui->video_display;
    video_display_sink = video_display_widget->videoSink();

    txt_video_filepath = ui->txt_video_filepath;
    btn_stop = ui->btn_stop;
    btn_start = ui->btn_start;
    pgb_video_process = ui->pgb_video_process;

    // ui->startButton;
    // ui->resumeButton;

    show_video_cover();

    picture_thread = std::thread([&] { loop_video_pictures(); });

    connect(btn_start, &QPushButton::clicked, this, &MainWindow::onStartBtnClicked);
    connect(btn_stop, &QPushButton::clicked, this, &MainWindow::onStopBtnClicked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::show_video_cover() {
    QImage cover("://resources/images/video_cover.png");
    QImage imageARBG = cover.convertToFormat(QImage::Format_ARGB32);

    display_arbg_image(imageARBG);
}

void MainWindow::loop_video_pictures() {
    while (true) {
        VideoPicture* pic = picture_factory.next();
        if (pic == nullptr)
            continue;

        auto frame_width = pic->Width();
        auto frame_height = pic->Height();
        cv::Mat mat(
            frame_height, frame_width, CV_8UC3, pic->frame->data[0], pic->frame->linesize[0]);

        // draw frame id
        std::string pic_id = fmt::format("{}", pic->id_);
        cv::Point posi{100, 100};
        int face = cv::FONT_HERSHEY_PLAIN;
        double scale = 2;
        cv::Scalar color{255, 0, 0}; // blue, BGR
        cv::putText(mat, pic_id, posi, face, scale, color, 2);

        auto t1 = std::chrono::steady_clock::now();
        // detect face
        std::shared_ptr<donde_toolkits::DetectResult> detect_result = face_pipeline.Detect(mat);
        for (auto& face : detect_result->faces) {
            if (face.confidence > 0.8) {
                cv::Rect box = face.box;
                // FIXME: save to first detected cards?
                auto cloned = mat.clone();
                auto size = cloned.size();
                if (box.x <= 0 || box.x >= size.width) {
                    continue;
                }
                if (box.y <= 0 || box.y >= size.height) {
                    continue;
                }
                if (box.x + box.width > size.width) {
                    std::cerr << "box width overflow!" << std::endl;
                    box.width = size.width - box.x;
                }
                if (box.y + box.height > size.height) {
                    std::cerr << "box height overflow!" << std::endl;
                    box.height = size.height - box.y;
                }
                detected_people_cards.at(0).images.push_back(human_card::CardImage{
                    .big_frame = cloned,
                    .small_face = cloned(box),
                    .face_rect = box,
                });
                // draw on orignal mat
                cv::rectangle(mat, box.tl(), box.br(), cv::Scalar(0, 255, 0));
            }
        }
        if (pic->Id() % 30 == 0) {
            auto [t2, used_ms] = time_since(t1);
            printf("face pipeline detect use time: %lld ms, pic_id: %ld, detected faces: %ld\n",
                   used_ms.count(),
                   pic->id_,
                   detect_result->faces.size());
        }

        ////  cv::Mat mat(pic.Height(), pic.Width(), CV_8UC3, pic.frame->data[0],
        /// pic.frame->linesize[0]);

        ////  // draw frame id
        ////  std::string picId = fmt::format("{}", pic.id_);
        ////  cv::Point posi{100, 100};
        ////  int face = cv::FONT_HERSHEY_PLAIN;
        ////  double scale = 2;
        ////  cv::Scalar color{255, 0, 0}; // blue, BGR
        ////  cv::putText(mat, picId, posi, face, scale, color, 2);

        ////  // detect face
        ////  std::shared_ptr<donde_toolkits::DetectResult> detect_result =
        /// face_pipeline.Detect(mat); /  for (auto& face : detect_result->faces) { /      if
        ///(face.confidence > 0.8) { /          cv::Rect box = face.box; / cv::rectangle(mat,
        /// box.tl(), box.br(), cv::Scalar(0, 255, 0)); /      } /  }

        ////  int progress = pic.id_ * 100 / video_total_frames;
        ////  pgb_video_process->setValue(progress);

        display_cv_image(mat);
    }
}

void MainWindow::display_cv_image(const cv::Mat& mat) {
    QImage imgBGR((uchar*)mat.data, mat.cols, mat.rows, QImage::Format_BGR888);
    QImage imageARBG = imgBGR.convertToFormat(QImage::Format_ARGB32);

    display_arbg_image(imageARBG);
}

void MainWindow::display_arbg_image(const QImage& imageARBG) {
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
        SetVideoDurationSeconds(info.duration_seconds);

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
void MainWindow::SetVideoTotalFrames(int64_t total_frames) { video_total_frames = total_frames; }

void MainWindow::update_detected_people() {
    auto area_vboxlayout = detected_people_area->layout();
    for (auto& child : area_vboxlayout->children()) {
        child->deleteLater();
    }

    for (int i = 0; i < detected_people_cards.size(); i++) {
        auto& card = detected_people_cards[i];
        auto sec = new ui::Section(QString::fromStdString(card.name), 300);
        connect(sec->toggleButton,
                &QToolButton::toggled,
                this,
                std::bind(&MainWindow::update_selected_section, this, i, _1));
        auto content = new QHBoxLayout();
        auto cover = QImage("://resources/images/video_cover.png");
        auto image_label = new QLabel();
        image_label->setPixmap(QPixmap::fromImage(cover));
        content->addWidget(image_label);
        sec->setContentLayout(*content);
        area_vboxlayout->addWidget(sec);
    }
}

void MainWindow::update_selected_section(int index, bool collapsed) {
    qDebug() << "index: " << index << ", collapsed: " << collapsed;
    int last_selected_people_card_index = selected_people_card_index;
    selected_people_card_index = index;
    // auto sections = detected_people_area->children();
    auto sections = detected_people_area->findChildren<QWidget*>();
    for (int i = 0; i < sections.size(); i++) {
        auto section = (ui::Section*)sections.at(i);
        if (i == selected_people_card_index) {
            qDebug() << "match one!";
            section->toggle(collapsed);
        } else if (i == last_selected_people_card_index) {
            qDebug() << "not match, collapse it!";
            section->toggle(false);
        }
        qDebug() << "updateHeights!";
        section->updateHeights();
    }
};
