#pragma once
#include <vector>
#include <functional>
#include <string>
#include <SDL.h>
#include "../Math.h"

struct MenuItem
{
    std::string text;
    Vector2 position;
    Vector2 size;
    bool isSelected;
    std::function<void()> onSelect;
};

class Menu
{
public:
    Menu(class Game* game);
    virtual ~Menu() = default;
    
    virtual void ProcessInput(const Uint8* keyState);
    virtual void Draw(class Renderer* renderer);
    
protected:
    class Game* mGame;
    int mSelectedIndex;
    std::vector<MenuItem> mMenuItems;
    float mMenuTimer;
    Uint32 mLastKeyPress;
};

