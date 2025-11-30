#include "Player.h"
#include "../Game.h"
#include "../Random.h"
#include "Enemy.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimatorComponent.h"
#include "../Components/ParticleSystemComponent.h"
#include "../Math.h"

Player::Player(class Game* game)
    : Actor(game)
    , mHealth(100.0f)
    , mMaxHealth(100.0f)
    , mMoveSpeed(300.0f)
    , mAttackCooldown(0.0f)
    , mDamageMultiplier(1.0f)
    , mAttackSpeedMultiplier(1.0f)
    , mProjectileCount(8)
    , mHasHealthRegen(false)
    , mHealthRegenRate(5.0f)
    , mProjectilePierce(0)
    , mCritChance(0.0f)
    , mCritMultiplier(2.0f)
    , mHasLifesteal(false)
    , mLifestealPercent(0.1f)
    , mExperienceMultiplier(1.0f)
    , mBounceProjectiles(false)
    , mShotgunMode(false)
    , mSpiralMode(false)
    , mOrbitalWeapons(false)
    , mOrbitalAngle(0.0f)
    , mOrbitalCount(4)
    , mReverseShot(false)
    , mHomingProjectiles(false)
    , mExplosiveProjectiles(false)
    , mExperience(0.0f)
    , mExperienceToNextLevel(100.0f)
    , mLevel(1)
    , mCurrentDirection(PlayerDirection::Front)
{
    float radius = 16.0f;
    
    mAnimatorComponent = new AnimatorComponent(
        this,
        "../Assets/Sprites/Player/Player.png",
        "../Assets/Sprites/Player/Player.json",
        28,  // Average width
        32   // Average height
    );
    
    mAnimatorComponent->AddAnimation("Back", {0, 2, 4});
    mAnimatorComponent->AddAnimation("Front", {1, 5, 6});
    mAnimatorComponent->AddAnimation("Right", {7, 8, 3});
    mAnimatorComponent->AddAnimation("Left", {7, 8, 3});
    
    mAnimatorComponent->AddAnimation("IdleFront", {1});
    mAnimatorComponent->AddAnimation("IdleBack", {0});
    mAnimatorComponent->AddAnimation("IdleRight", {7});
    mAnimatorComponent->AddAnimation("IdleLeft", {7});
    
    mAnimatorComponent->SetAnimation("IdleFront");
    mAnimatorComponent->SetAnimFPS(8.0f);  // 8 frames per second for walking animation

    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);

    SetRotation(0.0f);
}

void Player::OnProcessInput(const Uint8* state)
{
    Vector2 moveDirection(0.0f, 0.0f);

    if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP])    moveDirection.y -= 1.0f;
    if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN])  moveDirection.y += 1.0f;
    if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])  moveDirection.x -= 1.0f;
    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) moveDirection.x += 1.0f;

    if (moveDirection.LengthSq() > 0.01f)
    {
        moveDirection.Normalize();
        mRigidBodyComponent->SetVelocity(moveDirection * mMoveSpeed);

        UpdateAnimation(moveDirection);
    }
    else
    {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        
        switch (mCurrentDirection)
        {
            case PlayerDirection::Front:
                mAnimatorComponent->SetAnimation("IdleFront");
                break;
            case PlayerDirection::Back:
                mAnimatorComponent->SetAnimation("IdleBack");
                break;
            case PlayerDirection::Right:
                mAnimatorComponent->SetAnimation("IdleRight");
                SetScale(Vector2(1.0f, -1.0f));
                break;
            case PlayerDirection::Left:
                mAnimatorComponent->SetAnimation("IdleLeft");
                SetScale(Vector2(-1.0f, -1.0f));  // Flip horizontally
                break;
        }
    }
}

void Player::OnUpdate(float deltaTime)
{
    // Não atualiza se o jogo não está em estado Playing
    if (GetGame()->GetState() != MenuState::Playing)
    {
        return;
    }

    UpdateAutoAttack(deltaTime);

    // Orbitais (estético/jogabilidade)
    if (mOrbitalWeapons)
    {
        mOrbitalAngle += deltaTime * 3.0f;
        if (mOrbitalAngle > Math::TwoPi) mOrbitalAngle -= Math::TwoPi;

        Vector2 playerPos = GetPosition();
        float orbitRadius = 40.0f;
        static float orbitalTimer = 0.0f;
        orbitalTimer += deltaTime;
        if (orbitalTimer > 0.3f)
        {
            for (int i = 0; i < mOrbitalCount; ++i)
            {
                float angle = mOrbitalAngle + (Math::TwoPi / mOrbitalCount) * i;
                Vector2 orbitalPos = playerPos + Vector2(Math::Cos(angle) * orbitRadius, Math::Sin(angle) * orbitRadius);
                Vector2 tangentDir(-Math::Sin(angle), Math::Cos(angle)); // tangente
                // offset já está aplicado por nascer no perímetro do círculo orbital
                GetGame()->SpawnProjectile(orbitalPos, tangentDir, 400.0f * mDamageMultiplier, /*fromPlayer*/ true, /*damage*/ 16.0f * mDamageMultiplier);
            }
            orbitalTimer = 0.0f;
        }
    }

    // Regen
    if (mHasHealthRegen && mHealth < mMaxHealth)
    {
        Heal(mHealthRegenRate * deltaTime);
    }

    // Dano por contato com inimigos
    for (auto enemy : GetGame()->GetEnemies())
    {
        if (mCircleColliderComponent->Intersect(*enemy->GetComponent<CircleColliderComponent>()))
        {
            TakeDamage(10.0f * deltaTime);
            GetGame()->AddScreenShake(5.0f, 0.15f);
            break;
        }
    }

    if (mHealth <= 0.0f)
    {
        // Só chama GameOver se ainda estiver em estado Playing
        if (GetGame()->GetState() == MenuState::Playing)
        {
            GetGame()->GameOver();
        }
    }

    // Limites do mundo
    Vector2 pos = GetPosition();
    float radius = 16.0f;
    if (pos.x < radius) pos.x = radius;
    if (pos.x > Game::WORLD_WIDTH - radius) pos.x = Game::WORLD_WIDTH - radius;
    if (pos.y < radius) pos.y = radius;
    if (pos.y > Game::WORLD_HEIGHT - radius) pos.y = Game::WORLD_HEIGHT - radius;
    SetPosition(pos);
}

void Player::TakeDamage(float damage)
{
    mHealth -= damage;
    if (mHealth < 0.0f)
        mHealth = 0.0f;

    if (mHealth <= 0.0f)
    {
        if (GetGame()->GetState() == MenuState::Playing)
        {
            GetGame()->GameOver();
        }
    }
}


void Player::Heal(float amount)
{
    mHealth += amount;
    if (mHealth > mMaxHealth) mHealth = mMaxHealth;
}

void Player::AddExperience(float exp)
{
    mExperience += exp * mExperienceMultiplier;
    while (mExperience >= mExperienceToNextLevel)
    {
        mExperience -= mExperienceToNextLevel;
        mLevel++;
        mExperienceToNextLevel *= 1.5f;

        GetGame()->AddScreenShake(8.0f, 0.3f);
        GetGame()->ShowUpgradeMenu();
    }
}

void Player::UpdateAnimation(const Vector2& moveDirection)
{
    float absX = fabs(moveDirection.x);
    float absY = fabs(moveDirection.y);
    
    PlayerDirection newDirection = mCurrentDirection;
    
    if (absY > absX)
    {
        if (moveDirection.y < 0.0f)
        {
            newDirection = PlayerDirection::Back;
            mAnimatorComponent->SetAnimation("Back");
            SetScale(Vector2(1.0f, -1.0f));
        }
        else
        {
            newDirection = PlayerDirection::Front;
            mAnimatorComponent->SetAnimation("Front");
            SetScale(Vector2(1.0f, -1.0f));
        }
    }
    else
    {
        if (moveDirection.x > 0.0f)
        {
            newDirection = PlayerDirection::Right;
            mAnimatorComponent->SetAnimation("Right");
            SetScale(Vector2(1.0f, -1.0f));
        }
        else
        {
            newDirection = PlayerDirection::Left;
            mAnimatorComponent->SetAnimation("Left");
            SetScale(Vector2(-1.0f, -1.0f));
        }
    }
    
    mCurrentDirection = newDirection;
}

void Player::UpdateAutoAttack(float deltaTime)
{
    mAttackCooldown -= deltaTime;

    if (mAttackCooldown > 0.0f) return;

    // Recarrega cooldown
    float baseCooldown = 0.5f / mAttackSpeedMultiplier;
    mAttackCooldown = baseCooldown;

    Vector2 playerPos = GetPosition();
    const float playerRadius = 16.0f + 10.0f;
    const float baseSpeed   = 800.0f;               // velocidade padrão do projétil
    const float baseDamage  = 20.0f * mDamageMultiplier;

    // 1) Shotgun
    if (mShotgunMode)
    {
        int numProjectiles = mProjectileCount;
        float totalSpread = Math::Pi; // 180°
        for (int i = 0; i < numProjectiles; ++i)
        {
            float angle = (totalSpread / (numProjectiles - 1)) * i - totalSpread / 2.0f;
            Vector2 dir(Math::Cos(angle), Math::Sin(angle));
            dir.Normalize();
            Vector2 offset = dir * playerRadius;
            GetGame()->SpawnProjectile(playerPos + offset, dir, 1200.0f * mDamageMultiplier, /*fromPlayer*/ true, /*damage*/ 12.0f * mDamageMultiplier);
        }
        return;
    }

    // 2) Espiral
    if (mSpiralMode)
    {
        static float spiralAngle = 0.0f;
        spiralAngle += 0.5f;
        int numProjectiles = mProjectileCount;
        for (int i = 0; i < numProjectiles; ++i)
        {
            float angle = spiralAngle + (Math::TwoPi / numProjectiles) * i;
            Vector2 dir(Math::Cos(angle), Math::Sin(angle));
            dir.Normalize();
            Vector2 offset = dir * playerRadius;
            GetGame()->SpawnProjectile(playerPos + offset, dir, 800.0f * mDamageMultiplier, /*fromPlayer*/ true, /*damage*/ 16.0f * mDamageMultiplier);
        }
        return;
    }

    // 3) Padrão: mira no inimigo mais próximo; se não houver, atira em todas direções
    Enemy* nearestEnemy = nullptr;
    float nearestDistance = 10000.0f;

    for (auto enemy : GetGame()->GetEnemies())
    {
        float dist = (enemy->GetPosition() - playerPos).Length();
        if (dist < nearestDistance)
        {
            nearestDistance = dist;
            nearestEnemy = enemy;
        }
    }

    if (nearestEnemy)
    {
        Vector2 toEnemy = nearestEnemy->GetPosition() - playerPos;
        if (toEnemy.LengthSq() < 1e-6f) toEnemy = Vector2(1.0f, 0.0f);
        toEnemy.Normalize();

        Vector2 spawnOffset = toEnemy * playerRadius;

        int numProjectiles = mProjectileCount;
        float spreadAngle = 0.3f; // ~17°

        // Conjunto “leque” em torno do alvo
        for (int i = 0; i < numProjectiles; ++i)
        {
            float angleOffset = ((i - numProjectiles / 2.0f) / std::max(1, numProjectiles - 1)) * spreadAngle;
            float baseAngle = Math::Atan2(toEnemy.y, toEnemy.x);
            float current = baseAngle + angleOffset;

            Vector2 dir(Math::Cos(current), Math::Sin(current));
            dir.Normalize();

            Vector2 offset = dir * playerRadius;
            GetGame()->SpawnProjectile(playerPos + offset, dir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage);
        }

        // Reverse Shot opcional
        if (mReverseShot)
        {
            Vector2 reverseDir = toEnemy * -1.0f;
            reverseDir.Normalize();
            Vector2 reverseOffset = reverseDir * playerRadius;
            GetGame()->SpawnProjectile(playerPos + reverseOffset, reverseDir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage * 0.8f);
        }
    }
    else
    {
        // Sem inimigos: círculo completo
        int numProjectiles = mProjectileCount;
        for (int i = 0; i < numProjectiles; ++i)
        {
            float angle = (Math::TwoPi / numProjectiles) * i;
            Vector2 dir(Math::Cos(angle), Math::Sin(angle));
            dir.Normalize();

            Vector2 offset = dir * playerRadius;
            GetGame()->SpawnProjectile(playerPos + offset, dir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage);
        }
    }
}
