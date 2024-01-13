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

    detected_people_cards.insert({"alice",
                                  human_card::PeopleCard{.name = "Alice",
                                                         .images = {
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                         }}});

    detected_people_cards.insert({"bob",
                                  human_card::PeopleCard{.name = "Bob",
                                                         .images = {
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                         }}});

    detected_people_cards.insert({"candy",
                                  human_card::PeopleCard{.name = "Candy",
                                                         .images = {
                                                             {},
                                                             {},
                                                             {},
                                                         }}});

    un_classified_card_images.push_back(human_card::CardImage());
    un_classified_card_images.push_back(human_card::CardImage());
    un_classified_card_images.push_back(human_card::CardImage());
    un_classified_card_images.push_back(human_card::CardImage());

    detected_people_area = ui->scrollAreaWidgetContents;
    init_detected_card_images_area();

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
    QImage cover("://resources/images/1F3A5_color.png");
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

                un_classified_card_images.emplace_back(cloned, cloned(box), box);
                // draw on original mat
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

        QSize video_widget_size = video_display_widget->size();
        int height = video_widget_size.height();
        int width = video_widget_size.width();

        int new_width = info.width * 1.0 / info.height * height;
        video_display_widget->setFixedWidth(new_width);
        qDebug() << "video widget height: " << height << " width: " << width << ", new_width"
                 << new_width;
        qDebug() << "video meta height: " << info.height << " width: " << info.width;

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

void MainWindow::init_detected_card_images_area() {
    QVBoxLayout* area_vboxlayout = new QVBoxLayout();

    // if (detected_people_area->layout() != nullptr) {
    //     for (auto& child : detected_people_area->layout()->children()) {
    //         child->deleteLater();
    //     }
    // }

    // insert un-classified images
    {
        auto section = new ui::Section(QString::fromStdString("unclassified"));
        section->setObjectName("section-unclassified");

        auto contentLayout = new QHBoxLayout();
        for (auto& card_image : un_classified_card_images) {
            auto placeholder = QImage("://resources/images/OIP-C.jpg").scaled(100, 100);
            auto image_label = new QLabel();
            image_label->setPixmap(QPixmap::fromImage(placeholder));
            contentLayout->addWidget(image_label);
        }
        contentLayout->addStretch();

        section->setContentLayout(*contentLayout);
        area_vboxlayout->addWidget(section);
    }

    for (auto& [name, card] : detected_people_cards) {
        auto section = new ui::Section(QString::fromStdString(name));
        section->setObjectName("section-" + name);

        auto contentLayout = new QHBoxLayout();
        for (auto& img : card.images) {
            auto placeholder = QImage("://resources/images/1F9D1_color.png").scaled(100, 100);
            auto image_label = new QLabel();
            image_label->setPixmap(QPixmap::fromImage(placeholder));
            contentLayout->addWidget(image_label);
        }
        contentLayout->addStretch();

        section->setContentLayout(*contentLayout);
        area_vboxlayout->addWidget(section);
    }
    area_vboxlayout->setAlignment(Qt::AlignTop);
    detected_people_area->setLayout(area_vboxlayout);
    detected_people_area->children();
}
