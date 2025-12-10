#pragma once
#include "Actor.h"

class Background : public Actor
{
public:
    Background(class Game* game);
    ~Background();
    
    void Draw(class Renderer* renderer);
    
private:
    class Texture* mTexture;
    float mTileWidth;
    float mTileHeight;
};
