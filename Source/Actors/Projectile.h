#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    // Mantém compatibilidade: fromPlayer default = true, damage default = 10.0f
    Projectile(class Game* game,
               const Vector2& position,
               const Vector2& direction,
               float speed,
               bool fromPlayer = true,
               float damage = 10.0f);
    ~Projectile();

    void OnUpdate(float deltaTime) override;

    // NOVO: flags/acessores
    bool  IsFromPlayer() const { return mFromPlayer; }
    float GetDamage()   const { return mDamage; }
    void  SetDamage(float d)  { mDamage = d; }

private:
    Vector2 mDirection;
    float   mSpeed;
    float   mLifetime;

    bool  mFromPlayer;  // indica a origem do projétil
    float mDamage;

    class DrawComponent*        mDrawComponent;
    class RigidBodyComponent*   mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
};
