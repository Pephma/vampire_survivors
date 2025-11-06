#include "AudioSystem.h"
#include <SDL.h>
#include <SDL_log.h>

AudioSystem::AudioSystem()
    : mMusic(nullptr)
    , mInitialized(false)
{
}

AudioSystem::~AudioSystem()
{
    Shutdown();
}

bool AudioSystem::Initialize()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }

    mInitialized = true;
    return true;
}

void AudioSystem::Shutdown()
{
    if (mMusic)
    {
        Mix_FreeMusic(mMusic);
        mMusic = nullptr;
    }

    if (mInitialized)
    {
        Mix_CloseAudio();
        mInitialized = false;
    }
}

bool AudioSystem::LoadMusic(const std::string& fileName)
{
    if (mMusic)
    {
        Mix_FreeMusic(mMusic);
        mMusic = nullptr;
    }

    mMusic = Mix_LoadMUS(fileName.c_str());
    if (mMusic == nullptr)
    {
        SDL_Log("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }

    return true;
}

void AudioSystem::PlayMusic(int loops)
{
    if (mMusic)
    {
        if (Mix_PlayingMusic() == 0)
        {
            Mix_PlayMusic(mMusic, loops);
        }
    }
}

void AudioSystem::StopMusic()
{
    Mix_HaltMusic();
}

void AudioSystem::PauseMusic()
{
    if (Mix_PlayingMusic() == 1)
    {
        Mix_PauseMusic();
    }
}

void AudioSystem::ResumeMusic()
{
    if (Mix_PausedMusic() == 1)
    {
        Mix_ResumeMusic();
    }
}

bool AudioSystem::IsMusicPlaying() const
{
    return Mix_PlayingMusic() == 1;
}

