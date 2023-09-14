#include "mainwindow.h"
#include "video_context.h"
#include "picture_factory.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoContext video_ctx{"/tmp/Iron_Man-Trailer_HD.mp4"};
    MediaController controller{video_ctx};
    PictureFactory factory{video_ctx};

    MainWindow w{factory, controller};
    w.show();

    video_ctx.Process();
    return a.exec();
}
