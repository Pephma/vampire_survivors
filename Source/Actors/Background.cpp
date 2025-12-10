#include "Background.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Texture.h"
#include "../Math.h"

Background::Background(Game* game)
    : Actor(game)
    , mTileWidth(998.0f)
    , mTileHeight(635.0f)
{
    mTexture = game->GetRenderer()->GetTexture("../Assets/Sprites/Background/background.png");
}

Background::~Background()
{
}

void Background::Draw(Renderer* renderer)
{
    Vector2 cameraPos = GetGame()->GetCameraPosition();
    
    float screenWidth = 1024.0f;
    float screenHeight = 768.0f;
    
    int tilesX = (int)(screenWidth / mTileWidth) + 3;
    int tilesY = (int)(screenHeight / mTileHeight) + 3;
    
    float startX = cameraPos.x - (tilesX / 2.0f) * mTileWidth;
    float startY = cameraPos.y - (tilesY / 2.0f) * mTileHeight;
    
    startX = floor(startX / mTileWidth) * mTileWidth;
    startY = floor(startY / mTileHeight) * mTileHeight;
    
    for (int y = 0; y < tilesY; y++)
    {
        for (int x = 0; x < tilesX; x++)
        {
            Vector2 tilePos(
                startX + x * mTileWidth,
                startY + y * mTileHeight
            );
            
            Vector2 tileSize(mTileWidth, mTileHeight);
            
            renderer->DrawTexture(
                tilePos,
                tileSize,
                0.0f,                          
                Vector3(1.0f, 1.0f, 1.0f),     
                mTexture,
                Vector4(0.0f, 0.0f, 1.0f, 1.0f), 
                cameraPos,
                false,                         
                1.0f                           
            );
        }
    }
}
