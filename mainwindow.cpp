#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "3rdparty/flowlayout/flowlayout.h"
#include "Section.h"
#include "engine.h"
#include "libyuv.h"
#include "utils.h"

#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>
#include <__functional/bind.h>
#include <chrono>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace std::placeholders;

void clearLayout(QLayout* layout) {
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

MainWindow::MainWindow(PictureGenerator& picture_factory,
                       PlayController& media_controller,
                       FacePipeline& pipeline,
                       QSize windowSize,
                       QRect windowRect,
                       QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), media_controller(media_controller) {

    qDebug() << "windowSize: " << windowSize << ", windowRect: " << windowRect;
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, windowSize, windowRect));

    ui->setupUi(this);

    /*
    detected_people_cards.insert({"alice",
                                  peoplecards::PeopleCard{.name = "Alice",
                                                         .images = {
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                         }}});

    detected_people_cards.insert({"bob",
                                  peoplecards::PeopleCard{.name = "Bob",
                                                         .images = {
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                             {},
                                                         }}});

    detected_people_cards.insert({"candy",
                                  peoplecards::PeopleCard{.name = "Candy",
                                                         .images = {
                                                             {},
                                                             {},
                                                             {},
                                                         }}});
    */

    engine = std::make_shared<peoplecards::Engine>(picture_factory, media_controller, pipeline);

    btn_detect_video = ui->btn_detect_video;
    btn_detect_video->setText("🔥");

    btn_detect_report = ui->btn_detect_report;
    btn_detect_report->setText("💬");

    btn_settings = ui->btn_settings;
    btn_settings->setText("🎁");

    stack_pages = ui->stackedWidget;

    connect(btn_detect_video, &QPushButton::clicked, this, &MainWindow::onShowDetectVideoPage);
    connect(btn_detect_report, &QPushButton::clicked, this, &MainWindow::onShowDetectReportPage);
    connect(btn_settings, &QPushButton::clicked, this, &MainWindow::onShowSettingsPage);
    stack_pages->setCurrentIndex(0);

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

    connect(btn_start, &QPushButton::clicked, this, &MainWindow::onStartBtnClicked);
    connect(btn_stop, &QPushButton::clicked, this, &MainWindow::onStopBtnClicked);

    connect(engine.get(), &peoplecards::Engine::updateUI, this, &MainWindow::update_sections_if_need);
    connect(engine.get(), &peoplecards::Engine::displayCvMat, this, &MainWindow::display_cv_image);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::show_video_cover() {
    QImage cover("://resources/images/1F3A5_color.png");
    QImage imageARBG = cover.convertToFormat(QImage::Format_ARGB32);

    display_arbg_image(imageARBG);
}

bool MainWindow::display_cv_image(const cv::Mat& mat) {
    QImage imgBGR((uchar*)mat.data, mat.cols, mat.rows, QImage::Format_BGR888);
    QImage imageARBG = imgBGR.convertToFormat(QImage::Format_ARGB32);

    display_arbg_image(imageARBG);
    return true;
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
        qDebug() << "video widget height: " << height << " width: " << width << ", new_width" << new_width;
        qDebug() << "video meta height: " << info.height << " width: " << info.width;

        media_controller.Start();
        btn_start->setText("Pause");

        // peoplecards::ImageReceiverFunc image_receiver
        //     = std::bind(&MainWindow::display_cv_image, this, std::placeholders::_1);
        engine->Start();

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

void MainWindow::onShowDetectVideoPage() { stack_pages->setCurrentIndex(0); }
void MainWindow::onShowDetectReportPage() { stack_pages->setCurrentIndex(1); }
void MainWindow::onShowSettingsPage() { stack_pages->setCurrentIndex(2); }

void MainWindow::SetVideoOpenSuccess(bool succ) {
    video_open_success = succ;
    if (!video_open_success) {
        // TODO do what?
    }
}

void MainWindow::SetVideoDurationSeconds(int64_t duration) { video_duration_s = duration; }

void MainWindow::SetVideoTotalFrames(int64_t total_frames) { video_total_frames = total_frames; }

void MainWindow::init_detected_card_images_area() {
    QVBoxLayout* layout = new QVBoxLayout();

    auto& un_classified_card_images = engine->UnClassifiedCardImages();
    auto& detected_people_cards = engine->ClassifiedCardImages();

    // insert un-classified images
    {
        auto section_widget = create_section("unclassified", un_classified_card_images);
        layout->addWidget(section_widget);
    }

    // insert normal card image
    for (auto& [name, card] : detected_people_cards) {
        auto section_widget = create_section(name, card.images);
        layout->addWidget(section_widget);
    }

    layout->setAlignment(Qt::AlignTop);
    detected_people_area->setLayout(layout);
}

ui::Section* MainWindow::create_section(const std::string& name, const CardImageList& card_images) {
    auto section = new ui::Section(QString::fromStdString(name));

    auto widget_name = "section-" + name;
    section->setObjectName(widget_name);

    auto contentLayout = new FlowLayout();

    int i = 0;
    for (auto& img : card_images) {
        // FIXME: use real image
        // auto placeholder = QImage("://resources/images/1F9D1_color.png").scaled(100, 100);
        auto image_label = new QLabel();
        auto& mat = img.small_face;
        if (mat.empty()) {
            QImage placeholder("://resources/images/1F9D1_color.png");
            image_label->setPixmap(QPixmap::fromImage(placeholder.scaled(100, 100)));
        } else {
            // cv mat is bgr
            QImage imgRGB(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            image_label->setPixmap(QPixmap::fromImage(imgRGB.scaled(100, 100)));
        }
        QSizePolicy p{QSizePolicy::Fixed, QSizePolicy::Fixed};
        p.setRetainSizeWhenHidden(true);
        image_label->setSizePolicy(p);
        image_label->setObjectName("card-image-" + std::to_string(i));
        i++;
        contentLayout->addWidget(image_label);
    }

    contentLayout->setContentsMargins(0, 0, 0, 0);
    section->setContentLayout(*contentLayout);
    // connect(contentLayout, &FlowLayout::heightChanged, section, &ui::Section::setMinimumHeight);
    return section;
}

void MainWindow::update_section(const std::string& name, const CardImageList& card_images) {
    qDebug() << "update_section for " << name << ", image size: " << card_images.size();

    auto widget_name = "section-" + name;
    auto section = detected_people_area->findChild<ui::Section*>(widget_name.c_str());

    FlowLayout* layout = dynamic_cast<FlowLayout*>(section->getContentLayout());
    if (!layout) {
        std::cerr << "illegal state, section layout is not QHBoxLayout?? " << std::endl;
    }
    if (layout->count() == card_images.size()) {
        qDebug() << "card_images for " << name << " is unchanged, just return";
        return;
    }

    // clear old, add new
    clearLayout(layout);

    int i = 0;
    for (auto& img : card_images) {
        // FIXME: use real image
        // auto placeholder = QImage("://resources/images/1F9D1_color.png").scaled(100, 100);
        auto image_label = new QLabel();
        auto& mat = img.small_face;
        if (mat.empty()) {
            QImage placeholder("://resources/images/1F9D1_color.png");
            image_label->setPixmap(QPixmap::fromImage(placeholder.scaled(100, 100)));
        } else {
            // cv mat is bgr
            QImage imgRGB(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            image_label->setPixmap(QPixmap::fromImage(imgRGB.scaled(100, 100)));
        }
        QSizePolicy p{QSizePolicy::Fixed, QSizePolicy::Fixed};
        p.setRetainSizeWhenHidden(true);
        image_label->setSizePolicy(p);
        image_label->setObjectName("card-image-" + std::to_string(i));
        i++;
        layout->addWidget(image_label);
    }

    section->updateHeights();
}

void MainWindow::update_sections_if_need() {
    static auto t1 = std::chrono::steady_clock::now();
    auto [now, used_ms] = now_time_since(t1);
    if (used_ms.count() >= 1000) {
        qDebug() << "run update_sections_if_need";

        auto& un_classified_card_images = engine->UnClassifiedCardImages();
        auto& detected_people_cards = engine->ClassifiedCardImages();

        update_section("unclassified", un_classified_card_images);
        for (auto& [name, card] : detected_people_cards) {
            update_section(name, card.images);
        }
        t1 = now;
    }
}
