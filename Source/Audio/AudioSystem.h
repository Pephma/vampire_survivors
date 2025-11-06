#pragma once
#include <SDL_mixer.h>
#include <string>

class AudioSystem
{
public:
    AudioSystem();
    ~AudioSystem();

    bool Initialize();
    void Shutdown();

    bool LoadMusic(const std::string& fileName);
    void PlayMusic(int loops = -1);
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    bool IsMusicPlaying() const;

private:
    Mix_Music* mMusic;
    bool mInitialized;
};

