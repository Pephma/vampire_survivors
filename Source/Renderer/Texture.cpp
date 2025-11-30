#include "Texture.h"
#include <SDL.h>

Texture::Texture()
    : mTextureID(0)
      , mWidth(0)
      , mHeight(0) {
}

Texture::~Texture() {
}

bool Texture::Load(const std::string &filePath) {
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }
    SDL_Surface *surf = IMG_Load(filePath.c_str());
    if (!surf) {
        SDL_Log("Failed to load texture file %s", filePath.c_str());
        return false;
    }
    mWidth = surf->w;
    mHeight = surf->h;

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(surf);

    return true;
}

bool Texture::LoadFromSurface(SDL_Surface* surface) {
    if (!surface) {
        return false;
    }
    
    mWidth = surface->w;
    mHeight = surface->h;
    
    // Convert to RGBA if needed
    SDL_Surface* formattedSurface = nullptr;
    if (surface->format->format != SDL_PIXELFORMAT_RGBA32) {
        formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        if (!formattedSurface) {
            return false;
        }
        surface = formattedSurface;
    }
    
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if (formattedSurface) {
        SDL_FreeSurface(formattedSurface);
    }
    
    return true;
}

void Texture::Unload() {
    glDeleteTextures(1, &mTextureID);
}

void Texture::SetActive(int index) const {
    glBindTexture(GL_TEXTURE_2D, mTextureID);
}
