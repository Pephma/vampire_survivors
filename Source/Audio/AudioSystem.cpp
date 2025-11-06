#include "AudioSystem.h"
#include <SDL.h>
#include <SDL_log.h>

AudioSystem::AudioSystem()
    : mCurrentMusic(nullptr)
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
    for (auto& pair : mMusicMap)
    {
        if (pair.second)
        {
            Mix_FreeMusic(pair.second);
        }
    }
    mMusicMap.clear();
    mCurrentMusic = nullptr;

    if (mInitialized)
    {
        Mix_CloseAudio();
        mInitialized = false;
    }
}

bool AudioSystem::LoadMusic(const std::string& name, const std::string& fileName)
{
    if (mMusicMap.find(name) != mMusicMap.end())
    {
        Mix_FreeMusic(mMusicMap[name]);
    }

    std::string paths[] = {fileName, "../" + fileName, "../../" + fileName};
    Mix_Music* music = nullptr;

    for (const auto& path : paths)
    {
        music = Mix_LoadMUS(path.c_str());
        if (music != nullptr)
        {
            break;
        }
    }

    if (music == nullptr)
    {
        SDL_Log("Failed to load music %s! SDL_mixer Error: %s\n", fileName.c_str(), Mix_GetError());
        return false;
    }

    mMusicMap[name] = music;
    return true;
}

void AudioSystem::PlayMusic(const std::string& name, int loops)
{
    auto it = mMusicMap.find(name);
    if (it != mMusicMap.end())
    {
        mCurrentMusic = it->second;
        if (Mix_PlayingMusic() == 0)
        {
            Mix_PlayMusic(mCurrentMusic, loops);
        }
        else
        {
            Mix_HaltMusic();
            Mix_PlayMusic(mCurrentMusic, loops);
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

