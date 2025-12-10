#include "MainMenu.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include "../Renderer/TextRenderer.h"
#include "../Renderer/Texture.h"
#include "../Components/DrawComponent.h"

MainMenu::MainMenu(Game* game)
    : Menu(game)
    , mMusicStarted(false)
    , mBackgroundTexture(nullptr)
{
    mBackgroundTexture = game->GetRenderer()->GetTexture("../Assets/Menu/menu_background.png");
    MenuItem startItem;
    startItem.text = "Start Game";
    startItem.position = Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 100.0f, static_cast<float>(Game::WINDOW_HEIGHT) / 2.0f);
    startItem.size = Vector2(200.0f, 40.0f);
    startItem.onSelect = [this]() {
        mGame->StartNewGame();
    };
    mMenuItems.push_back(startItem);
    
    MenuItem quitItem;
    quitItem.text = "Quit";
    quitItem.position = Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 100.0f, static_cast<float>(Game::WINDOW_HEIGHT) / 2.0f + 60.0f);
    quitItem.size = Vector2(200.0f, 40.0f);
    quitItem.onSelect = [this]() {
        mGame->Quit();
    };
    mMenuItems.push_back(quitItem);
}

void MainMenu::Draw(Renderer* renderer)
{
    if (mGame->GetAudioSystem())
    {
        if (!mGame->GetAudioSystem()->IsMusicPlaying())
        {
            mGame->GetAudioSystem()->PlayMusic("menu", -1);
            mMusicStarted = true;
        }
    }

    if (mBackgroundTexture)
    {
        Vector4 flippedRect(0.0f, 1.0f, 1.0f, -1.0f);
        renderer->DrawTexture(Vector2(0.0f, 0.0f),
                            Vector2(static_cast<float>(Game::WINDOW_WIDTH), 
                                   static_cast<float>(Game::WINDOW_HEIGHT)),
                            0.0f,
                            Vector3(1.0f, 1.0f, 1.0f),
                            mBackgroundTexture,
                            flippedRect,
                            Vector2::Zero,
                            false,
                            1.0f);
    }

    Menu::Draw(renderer);
    
    std::vector<Vector2> titleBg;
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 150.0f, 100.0f));
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f + 150.0f, 100.0f));
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f + 150.0f, 180.0f));
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 150.0f, 180.0f));
    
    std::vector<float> titleFloatArray;
    std::vector<unsigned int> titleIndices;
    for (size_t i = 0; i < titleBg.size(); ++i)
    {
        titleFloatArray.push_back(titleBg[i].x);
        titleFloatArray.push_back(titleBg[i].y);
        titleFloatArray.push_back(0.0f);
        titleIndices.push_back(static_cast<unsigned int>(i));
    }
    
    Matrix4 titleMatrix = Matrix4::Identity;
    VertexArray titleVA(titleFloatArray.data(), static_cast<unsigned int>(titleBg.size()), titleIndices.data(), static_cast<unsigned int>(titleIndices.size()));
    Vector3 titleColor(0.8f, 0.2f, 0.2f);
    renderer->Draw(titleMatrix, &titleVA, titleColor);
    
    // Draw title text
    Vector2 titleTextPos(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 140.0f, 130.0f);
    Vector3 titleTextColor(1.0f, 1.0f, 1.0f);
    TextRenderer::DrawText(renderer, "Foge, S\xF4!", titleTextPos, 2.0f, titleTextColor);
}

