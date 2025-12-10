#pragma once
#include "Menu.h"

class MainMenu : public Menu
{
public:
    MainMenu(class Game* game);
    void Draw(class Renderer* renderer) override;

private:
    bool mMusicStarted;
    class Texture* mBackgroundTexture;
};

