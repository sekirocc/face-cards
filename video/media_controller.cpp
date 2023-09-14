#include "media_controller.h"

MediaController::MediaController(VideoContext &video_ctx) : video_ctx{video_ctx} {};

const MediaPlayState &MediaController::CurrentState() const
{
    return state;
}

bool MediaController::Reload(std::string filename)
{
    return true;
}

bool MediaController::Pause()
{
    return true;
}

bool MediaController::Resume()
{
    return true;
}

bool MediaController::Terminate()
{
    return true;
}

bool MediaController::Forward(int step)
{
    return true;
}

bool MediaController::Backward(int step)
{
    return true;
}

bool MediaController::Seek(int position)
{
    return true;
}
