#pragma once
#include "Actor.h"

class FloatingText : public Actor
{
public:
    FloatingText(class Game* game, const Vector2& position, const std::string& text, const Vector3& color, float lifetime = 1.5f, float speed = 80.0f);
    
    void OnUpdate(float deltaTime) override;
    
private:
    float mLifetime;
    float mMaxLifetime;
    float mSpeed;
    std::string mText;
    Vector3 mColor;
    class DrawComponent* mDrawComponent;
};

