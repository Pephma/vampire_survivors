#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    Projectile(class Game* game, const Vector2& position, const Vector2& direction, float speed);
    ~Projectile();
    
    void OnUpdate(float deltaTime) override;
    
private:
    Vector2 mDirection;
    float mSpeed;
    float mLifetime;
    
    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
};
