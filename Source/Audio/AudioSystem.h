#pragma once
#include <SDL_mixer.h>
#include <string>
#include <map>

class AudioSystem
{
public:
    AudioSystem();
    ~AudioSystem();

    bool Initialize();
    void Shutdown();

    bool LoadMusic(const std::string& name, const std::string& fileName);
    void PlayMusic(const std::string& name, int loops = -1);
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    bool IsMusicPlaying() const;

private:
    std::map<std::string, Mix_Music*> mMusicMap;
    Mix_Music* mCurrentMusic;
    bool mInitialized;
};

