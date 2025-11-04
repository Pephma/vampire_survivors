#include "PauseMenu.h"
#include "../Game.h"

PauseMenu::PauseMenu(Game* game)
    : Menu(game)
{
    MenuItem resumeItem;
    resumeItem.text = "Resume";
    resumeItem.position = Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 100.0f, static_cast<float>(Game::WINDOW_HEIGHT) / 2.0f - 40.0f);
    resumeItem.size = Vector2(200.0f, 40.0f);
    resumeItem.onSelect = [this]() {
        mGame->ResumeGame();
    };
    mMenuItems.push_back(resumeItem);
    
    MenuItem quitItem;
    quitItem.text = "Quit to Menu";
    quitItem.position = Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 100.0f, static_cast<float>(Game::WINDOW_HEIGHT) / 2.0f + 40.0f);
    quitItem.size = Vector2(200.0f, 40.0f);
    quitItem.onSelect = [this]() {
        mGame->QuitToMenu();
    };
    mMenuItems.push_back(quitItem);
}

