#pragma once
#include "Actor.h"

class DelayedExplosion : public Actor
{
public:
    DelayedExplosion(class Game* game, Vector2 position, float delay, float radius);
    void OnUpdate(float deltaTime) override;

private:
    float mTimer; // Temporizador regressivo
    float mRadius;
};