#pragma once
#include <sqstd/thread/sqtaskthread.h>
#include <string>
#include <vector>


namespace snqu{

    class SoundPlayer 
    {
    public:
        SoundPlayer();
        ~SoundPlayer();

        int play_sound(const std::string& file_name);
        int play_sounds(const std::vector<std::string>& params);

		// 初始化音频文件路径，audio_dir为相对路径，路径后面一定要加"\\"
        void init(const std::string& audio_dir);
        void uninit();

    private:
        SoundPlayer(const SoundPlayer&);
        SoundPlayer& operator=(const SoundPlayer&);

        std::string get_sound_file_dir(const std::string &name);

        TaskThread<int> m_thread;

        std::string m_voice_file_root;
        bool is_uninit;
    };
}