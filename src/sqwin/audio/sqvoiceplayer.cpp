#include <sqwin/audio/sqvoiceplayer.h>
#include <sqwin/win/sqwindows.h>
#include <sqwin/win/sqpath.h>
#include <MMSystem.h>                // 声音头文件
#pragma comment(lib, "winmm.lib")    // 声音库文件

namespace snqu {


SoundPlayer::SoundPlayer()
    : is_uninit(false)
{}

SoundPlayer::~SoundPlayer()
{
}

void SoundPlayer::init(const std::string& audio_dir)
{
    m_voice_file_root = audio_dir;
}

void SoundPlayer::uninit()
{
    stop_play();
}

void SoundPlayer::stop_play()
{
    ::PlaySound(NULL, NULL, SND_FILENAME | SND_PURGE);
}

std::string SoundPlayer::get_sound_file_dir(const std::string &name)
{
    std::string file_path = path::get_module_path() + m_voice_file_root;
    file_path.append(name);
    if (std::string::npos == name.find("."))
        file_path.append(".wav");
    return std::move(file_path);
}

int SoundPlayer::play_sound(const std::string& file_name)
{
    std::string file_path = get_sound_file_dir(file_name);
    if(!::PlaySoundA(file_path.c_str(), NULL, SND_FILENAME | SND_NODEFAULT | SND_NOWAIT | SND_NOSTOP | SND_ASYNC))
    {
        //SNLOG(kFatal, << "sound " << file_path << " play failed err:" << GetLastError();
        return -1;
    }
    
    return 0;
}

int SoundPlayer::play_sounds(const std::vector<std::string>& params)
{
    std::string file_path;

    for (auto& item : params)
    {
        file_path = get_sound_file_dir(item);
        if(!::PlaySoundA(file_path.c_str(), NULL, SND_FILENAME | SND_NODEFAULT | SND_SYNC))
        {
            //SNLOG(kFatal, << "sound " << file_path << " play failed err:" << GetLastError();
            return -1;
        }
    }

    return 0;
}

}