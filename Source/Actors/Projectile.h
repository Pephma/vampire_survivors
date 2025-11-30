#pragma once
#include "Actor.h"
#include <set>

class Projectile : public Actor
{
public:
    // Mantém compatibilidade: fromPlayer default = true, damage default = 10.0f
    Projectile(class Game* game,
               const Vector2& position,
               const Vector2& direction,
               float speed,
               bool fromPlayer = true,
               float damage = 10.0f,
               int pierce = 0,
               bool homing = false,
               bool explosive = false);
    ~Projectile();

    void OnUpdate(float deltaTime) override;

    // NOVO: flags/acessores
    bool  IsFromPlayer() const { return mFromPlayer; }
    float GetDamage()   const { return mDamage; }
    void  SetDamage(float d)  { mDamage = d; }
    int   GetPierce() const { return mPierceRemaining; }
    bool  IsHoming() const { return mHoming; }
    bool  IsExplosive() const { return mExplosive; }

private:
    void UpdateHoming(float deltaTime);
    void CreateExplosion(const Vector2& position);
    
    Vector2 mDirection;
    float   mSpeed;
    float   mLifetime;

    bool  mFromPlayer;  // indica a origem do projétil
    float mDamage;
    
    // Pierce system
    int mPierceRemaining;
    std::set<class Enemy*> mHitEnemies;  // Track enemies already hit
    
    // Homing system
    bool mHoming;
    float mHomingTurnRate;  // How fast projectile turns toward target
    
    // Explosive system
    bool mExplosive;
    float mExplosionRadius;

    class DrawComponent*        mDrawComponent;
    class RigidBodyComponent*   mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
};
