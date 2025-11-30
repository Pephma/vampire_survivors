#include "TextRenderer.h"
#include "../Game.h"
#include "Texture.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include <GL/glew.h>

TTF_Font* TextRenderer::sFont = nullptr;
bool TextRenderer::sInitialized = false;

bool TextRenderer::Initialize()
{
    if (sInitialized)
        return true;
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }
    
    // Try to load fonts from Assets/Fonts directory (relative to executable)
    const char* fontPaths[] = {
        "Assets/Fonts/PressStart2P-Regular.ttf",  // Pixel font - great for games
        "Assets/Fonts/font.ttf",                   // Generic fallback name
        "../Assets/Fonts/PressStart2P-Regular.ttf", // Try parent directory
        "../../Assets/Fonts/PressStart2P-Regular.ttf", // Try build directory
        nullptr
    };
    
    int fontSize = 24; // Base size, will be scaled
    for (int i = 0; fontPaths[i] != nullptr; ++i)
    {
        sFont = TTF_OpenFont(fontPaths[i], fontSize);
        if (sFont != nullptr)
        {
            SDL_Log("Loaded font: %s", fontPaths[i]);
            sInitialized = true;
            return true;
        }
    }
    
    // If no font found, log warning but continue
    SDL_Log("Warning: Could not load font from Assets/Fonts/. Text will not render.");
    sInitialized = true; // Mark as initialized even without font
    return true;
}

void TextRenderer::Shutdown()
{
    if (sFont)
    {
        TTF_CloseFont(sFont);
        sFont = nullptr;
    }
    
    if (sInitialized)
    {
        TTF_Quit();
        sInitialized = false;
    }
}

void TextRenderer::DrawText(Renderer* renderer, const std::string& text, const Vector2& position, float scale, const Vector3& color)
{
    if (!sInitialized)
    {
        if (!Initialize())
        {
            return;
        }
    }
    
    if (!sFont)
    {
        return;
    }
    
    // Set font size based on scale - reduced base size for better UI scaling
    int fontSize = static_cast<int>(10.0f * scale);
    TTF_SetFontSize(sFont, fontSize);
    
    // Create text surface
    SDL_Color sdlColor;
    sdlColor.r = static_cast<Uint8>(color.x * 255.0f);
    sdlColor.g = static_cast<Uint8>(color.y * 255.0f);
    sdlColor.b = static_cast<Uint8>(color.z * 255.0f);
    sdlColor.a = 255;
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(sFont, text.c_str(), sdlColor);
    if (!textSurface)
    {
        SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    
    Vector2 size(static_cast<float>(textSurface->w), static_cast<float>(textSurface->h));
    
    // Create a Texture object from the surface
    Texture textTexture;
    if (!textTexture.LoadFromSurface(textSurface))
    {
        SDL_Log("Unable to create texture from text surface!");
        SDL_FreeSurface(textSurface);
        return;
    }
    
    // For screen-space UI rendering:
    // DrawTexture calculates: finalPos = position + (screenCenter - cameraPos)
    // We want finalPos = position (the screen coordinate passed in)
    // So: position = position + screenCenter - cameraPos
    // Therefore: cameraPos = screenCenter
    // And we pass position directly (already in screen coordinates)
    Vector2 screenCenter(1024.0f / 2.0f, 768.0f / 2.0f);
    
    // The sprite vertices are centered at (0,0), so the texture is drawn centered at the position.
    // The position passed in is the top-left corner where text should be.
    // To make the sprite render with its top-left at the position, we need to offset by half the size.
    // Since sprite center is at position, and we want top-left at position:
    // sprite top-left = position - size/2, so we need center = position + size/2
    Vector2 centeredPos = position + Vector2(size.x * 0.5f, size.y * 0.5f);
    
    // Flip texture coordinates vertically because SDL_Surface is top-to-bottom
    // but OpenGL expects bottom-to-top. TextureRect format: (x, y, width, height)
    // We flip by using y=1.0f and height=-1.0f to reverse the V coordinate
    Vector4 textureRect(0.0f, 1.0f, 1.0f, -1.0f);
    
    // Pass centered position (screen coordinates) and cameraPos = screenCenter
    // This makes finalPos = centeredPos + screenCenter - screenCenter = centeredPos ✓
    renderer->DrawTexture(centeredPos, size, 0.0f, Vector3(1.0f, 1.0f, 1.0f), 
                         &textTexture, textureRect, screenCenter, false, 1.0f);
    
    // Clean up
    textTexture.Unload();
    SDL_FreeSurface(textSurface);
}
