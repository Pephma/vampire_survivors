#pragma once
#include "Actor.h"

class ExperienceOrb : public Actor
{
public:
    ExperienceOrb(class Game* game, const Vector2& position, float experienceValue);
    ~ExperienceOrb();

    void OnUpdate(float deltaTime) override;
    
    float GetExperienceValue() const { return mExperienceValue; }
    bool IsCollected() const { return mCollected; }
    void SetCollected(bool collected) { mCollected = collected; }

private:
    float mExperienceValue;
    float mLifetime;
    float mFloatPhase;
    bool mCollected;
    
    class DrawComponent* mDrawComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    class RigidBodyComponent* mRigidBodyComponent;
};

