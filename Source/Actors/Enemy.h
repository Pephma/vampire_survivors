#pragma once
#include "Actor.h"
#include "../Game.h"

enum class EnemyDirection
{
    Front,
    Back,
    Right,
    Left
};

class Enemy : public Actor
{
public:
    Enemy(class Game* game, EnemyKind kind, float radius, float speed, float health);
    ~Enemy();

    void OnUpdate(float deltaTime) override;

    // --- getters básicos ---
    float GetHealth() const { return mHealth; }
    float GetRadius() const { return mRadius; }
    float GetMaxHealth() const { return mMaxHealth; }

    // --- atributos expostos/setters usados pelo Game ---
    void SetSpeed(float s) { mSpeed = s; }
    void SetDamage(float d) { mDamage = d; }
    void SetExperienceValue(float xp) { mExperienceValue = xp; }

    void SetExplodesOnDeath(bool v) { mExplodesOnDeath = v; }
    void SetExplosionDamage(float d) { mExplosionDamage = d; }
    void SetExplosionRadius(float r) { mExplosionRadius = r; }

    void SetRangedShooter(bool v, float shootEvery)
    {
        mIsRangedShooter = v;
        mShootEvery = shootEvery;
        if (mShootEvery < 0.1f) mShootEvery = 0.1f;
    }
    void SetShootEvery(float s) { mShootEvery = (s < 0.1f ? 0.1f : s); }
    void SetProjectileSpeed(float v) { mProjectileSpeed = v; }

    // util
    void SetColor(const Vector3& c);

    // dano recebido retorna quanto foi de fato aplicado
    float TakeDamage(float damage);

protected:
    void ChasePlayer(float deltaTime);
    void UpdateAnimation(const Vector2& directionToPlayer);

    // Variáveis que o Boss precisa
    float mHealth;
    float mMaxHealth;
    float mExperienceValue = 10.0f;

    float mSpeed;

    // Componentes que o Boss precisa
    class RigidBodyComponent* mRigidBodyComponent;
    class AnimatorComponent* mAnimatorComponent;
    class CircleColliderComponent* mCircleColliderComponent;

    class DrawComponent* mDrawComponent;
    EnemyKind mKind;
    EnemyDirection mCurrentDirection;

private:
    void TryShootAtPlayer(float deltaTime);
    void DoDeathExplosion(); // usado para o Gordo Explosivo


    float mRadius;

    // novo: combate
    float mDamage = 10.0f;          // dano por contato ou base para inimigos

    // explosão (Gordo Explosivo)
    bool  mExplodesOnDeath = false;
    float mExplosionDamage = 40.0f;
    float mExplosionRadius = 150.0f;

    // ataque à distância (Atirador)
    bool  mIsRangedShooter = false;
    float mShootEvery = 2.0f;     // intervalo entre tiros
    float mShootTimer = 0.0f;     // contador regressivo
    float mProjectileSpeed = 500.0f;

    bool mWasCritKilled;

};
