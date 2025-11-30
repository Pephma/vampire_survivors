#pragma once
#include "../Math.h"
#include "Renderer.h"
#include <string>
#include <SDL_ttf.h>

class TextRenderer
{
public:
    static bool Initialize();
    static void Shutdown();
    static void DrawText(Renderer* renderer, const std::string& text, const Vector2& position, float scale, const Vector3& color);
    
private:
    static TTF_Font* sFont;
    static bool sInitialized;
};
