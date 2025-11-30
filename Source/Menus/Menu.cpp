#include "Menu.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include "../Renderer/TextRenderer.h"
#include "../Components/DrawComponent.h"
#include <SDL.h>

Menu::Menu(Game* game)
    : mGame(game)
    , mSelectedIndex(0)
    , mMenuTimer(0.0f)
    , mLastKeyPress(0)
{
}

void Menu::ResetInput()
{
    mLastKeyPress = SDL_GetTicks();
}

void Menu::ProcessInput(const Uint8* keyState)
{
    Uint32 currentTime = SDL_GetTicks();
    
    // Check if keys were just pressed (not held) with debouncing
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
    {
        if (currentTime - mLastKeyPress > 200) // 200ms delay
        {
            mSelectedIndex--;
            if (mSelectedIndex < 0)
                mSelectedIndex = static_cast<int>(mMenuItems.size() - 1);
            mLastKeyPress = currentTime;
        }
    }
    else if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
    {
        if (currentTime - mLastKeyPress > 200) // 200ms delay
        {
            mSelectedIndex++;
            if (mSelectedIndex >= static_cast<int>(mMenuItems.size()))
                mSelectedIndex = 0;
            mLastKeyPress = currentTime;
        }
    }
    else if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_SPACE])
    {
        if (currentTime - mLastKeyPress > 200) // 200ms delay
        {
            if (mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mMenuItems.size()))
            {
                if (mMenuItems[mSelectedIndex].onSelect)
                {
                    mMenuItems[mSelectedIndex].onSelect();
                }
            }
            mLastKeyPress = currentTime;
        }
    }
    else
    {
        // Reset timer when no keys are pressed for a while
        if (currentTime - mLastKeyPress > 500)
        {
            mLastKeyPress = 0;
        }
    }
}

void Menu::Draw(Renderer* renderer)
{
    // Draw menu background
    std::vector<Vector2> bgVertices;
    bgVertices.emplace_back(Vector2(0.0f, 0.0f));
    bgVertices.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH), 0.0f));
    bgVertices.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));
    bgVertices.emplace_back(Vector2(0.0f, static_cast<float>(Game::WINDOW_HEIGHT)));
    
    // Convert to float array
    std::vector<float> bgFloatArray;
    std::vector<unsigned int> bgIndices;
    for (size_t i = 0; i < bgVertices.size(); ++i)
    {
        bgFloatArray.push_back(bgVertices[i].x);
        bgFloatArray.push_back(bgVertices[i].y);
        bgFloatArray.push_back(0.0f);
        bgIndices.push_back(static_cast<unsigned int>(i));
    }
    
    Matrix4 bgMatrix = Matrix4::Identity;
    VertexArray bgVA(bgFloatArray.data(), static_cast<unsigned int>(bgVertices.size()), bgIndices.data(), static_cast<unsigned int>(bgIndices.size()));
    Vector3 bgColor(0.1f, 0.1f, 0.15f);
    renderer->Draw(bgMatrix, &bgVA, bgColor);
    
    // Draw menu items
    for (size_t i = 0; i < mMenuItems.size(); ++i)
    {
        const auto& item = mMenuItems[i];
        bool isSelected = (static_cast<int>(i) == mSelectedIndex);
        
        Vector3 color = isSelected ? Vector3(1.0f, 0.8f, 0.0f) : Vector3(0.7f, 0.7f, 0.7f);
        
        // Draw selection indicator
        if (isSelected)
        {
            std::vector<Vector2> selVertices;
            selVertices.emplace_back(Vector2(item.position.x - 10.0f, item.position.y));
            selVertices.emplace_back(Vector2(item.position.x - 5.0f, item.position.y + 5.0f));
            selVertices.emplace_back(Vector2(item.position.x - 5.0f, item.position.y - 5.0f));
            
            std::vector<float> selFloatArray;
            std::vector<unsigned int> selIndices;
            for (size_t j = 0; j < selVertices.size(); ++j)
            {
                selFloatArray.push_back(selVertices[j].x);
                selFloatArray.push_back(selVertices[j].y);
                selFloatArray.push_back(0.0f);
                selIndices.push_back(static_cast<unsigned int>(j));
            }
            
            Matrix4 selMatrix = Matrix4::Identity;
            VertexArray selVA(selFloatArray.data(), static_cast<unsigned int>(selVertices.size()), selIndices.data(), static_cast<unsigned int>(selIndices.size()));
            renderer->Draw(selMatrix, &selVA, color);
        }
        
        // Draw button background
        std::vector<Vector2> btnVertices;
        btnVertices.emplace_back(Vector2(item.position.x, item.position.y));
        btnVertices.emplace_back(Vector2(item.position.x + item.size.x, item.position.y));
        btnVertices.emplace_back(Vector2(item.position.x + item.size.x, item.position.y + item.size.y));
        btnVertices.emplace_back(Vector2(item.position.x, item.position.y + item.size.y));
        
        std::vector<float> btnFloatArray;
        std::vector<unsigned int> btnIndices;
        for (size_t j = 0; j < btnVertices.size(); ++j)
        {
            btnFloatArray.push_back(btnVertices[j].x);
            btnFloatArray.push_back(btnVertices[j].y);
            btnFloatArray.push_back(0.0f);
            btnIndices.push_back(static_cast<unsigned int>(j));
        }
        
        Matrix4 btnMatrix = Matrix4::Identity;
        VertexArray btnVA(btnFloatArray.data(), static_cast<unsigned int>(btnVertices.size()), btnIndices.data(), static_cast<unsigned int>(btnIndices.size()));
        Vector3 btnColor = isSelected ? Vector3(0.3f, 0.3f, 0.4f) : Vector3(0.2f, 0.2f, 0.3f);
        renderer->Draw(btnMatrix, &btnVA, btnColor);
        
        // Draw text
        Vector2 textPos(item.position.x + 10.0f, item.position.y + item.size.y / 2.0f - 5.0f);
        TextRenderer::DrawText(renderer, item.text, textPos, 1.5f, color);
    }
}

