#pragma once
#include "Actor.h"

class Enemy : public Actor
{
public:
    Enemy(class Game* game, float radius, float speed, float health);
    ~Enemy();
    
    void OnUpdate(float deltaTime) override;
    
    float GetHealth() const { return mHealth; }
    float TakeDamage(float damage); // Returns actual damage dealt
    float GetRadius() const { return mRadius; }
    
private:
    float mHealth;
    float mMaxHealth;
    float mSpeed;
    float mRadius;
    bool mWasCritKilled;
    
    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    
    void ChasePlayer(float deltaTime);
};
