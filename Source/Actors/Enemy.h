#pragma once
#include "Actor.h"

class Enemy : public Actor
{
public:
    Enemy(class Game* game, float radius, float speed, float health);
    ~Enemy();

    void OnUpdate(float deltaTime) override;
    float TakeDamage(float damage);
    float GetHealth() const { return mHealth; }
    float GetRadius() const { return mRadius; }

    // ========= NOVOS MÉTODOS =========
    void SetColor(const Vector3& color);
    void SetExplodesOnDeath(bool v) { mExplodesOnDeath = v; }
    void SetRangedShooter(bool v, float every = 0.0f) { mRangedShooter = v; mShootEvery = every; }
    // =================================

private:
    float mHealth;
    float mMaxHealth;
    float mSpeed;
    float mRadius;
    bool mWasCritKilled;

    bool  mExplodesOnDeath = false;
    bool  mRangedShooter   = false;
    float mShootEvery      = 0.0f;
    float mShootTimer      = 0.0f;

    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;

    void ChasePlayer(float deltaTime);
};
