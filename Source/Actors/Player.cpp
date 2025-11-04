#include "Player.h"
#include "../Game.h"
#include "../Random.h"
#include "Enemy.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
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
{
    // Create a simple character shape (circle with a small triangle on top for direction)
    float radius = 15.0f;
    std::vector<Vector2> vertices;
    
    // Create circular body - use enough vertices for a visible circle
    int numVertices = 16;
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * radius, Math::Sin(angle) * radius));
    }
    
    // Add a small triangle pointing up to show direction
    vertices.emplace_back(Vector2(0.0f, radius + 8.0f));
    vertices.emplace_back(Vector2(-5.0f, radius));
    vertices.emplace_back(Vector2(5.0f, radius));
    
    mDrawComponent = new DrawComponent(this, vertices);
    // Super cool player color - bright cyan with glow
    Vector3 playerColor = Vector3(0.0f, 1.0f, 1.0f); // Bright cyan
    mDrawComponent->SetColor(playerColor);
    mDrawComponent->SetFilled(true); // Filled for better visibility
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);
    
    // Don't rotate player - we move in direction instead
    SetRotation(0.0f);
}

void Player::OnProcessInput(const Uint8* state)
{
    Vector2 moveDirection(0.0f, 0.0f);
    
    // WASD movement
    if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP])
    {
        moveDirection.y -= 1.0f;
    }
    if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN])
    {
        moveDirection.y += 1.0f;
    }
    if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
    {
        moveDirection.x -= 1.0f;
    }
    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
    {
        moveDirection.x += 1.0f;
    }
    
    // Normalize diagonal movement
    if (moveDirection.LengthSq() > 0.01f)
    {
        moveDirection.Normalize();
        mRigidBodyComponent->SetVelocity(moveDirection * mMoveSpeed);
        
        // Rotate character to face movement direction
        float angle = Math::Atan2(moveDirection.y, moveDirection.x) + Math::PiOver2;
        SetRotation(angle);
    }
    else
    {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }
}

void Player::OnUpdate(float deltaTime)
{
    UpdateAutoAttack(deltaTime);
    
    // Update orbital weapons
    if (mOrbitalWeapons)
    {
        mOrbitalAngle += deltaTime * 3.0f; // Rotate orbitals
        if (mOrbitalAngle > Math::TwoPi)
            mOrbitalAngle -= Math::TwoPi;
        
        // Spawn orbital projectiles that orbit around player
        Vector2 playerPos = GetPosition();
        float orbitRadius = 40.0f;
        static float orbitalTimer = 0.0f;
        orbitalTimer += deltaTime;
        if (orbitalTimer > 0.3f) // Spawn orbitals periodically
        {
            for (int i = 0; i < mOrbitalCount; ++i)
            {
                float angle = mOrbitalAngle + (Math::TwoPi / mOrbitalCount) * i;
                Vector2 orbitalPos = playerPos + Vector2(Math::Cos(angle) * orbitRadius, Math::Sin(angle) * orbitRadius);
                Vector2 tangentDir(-Math::Sin(angle), Math::Cos(angle)); // Perpendicular to radius
                GetGame()->SpawnProjectile(orbitalPos, tangentDir, 400.0f * mDamageMultiplier);
            }
            orbitalTimer = 0.0f;
        }
    }
    
    // Health regen
    if (mHasHealthRegen && mHealth < mMaxHealth)
    {
        Heal(mHealthRegenRate * deltaTime);
    }
    
    // Check collision with enemies
    for (auto enemy : GetGame()->GetEnemies())
    {
        if (mCircleColliderComponent->Intersect(*enemy->GetComponent<CircleColliderComponent>()))
        {
            TakeDamage(10.0f * deltaTime); // Damage over time on collision
            // Screen shake on taking damage
            GetGame()->AddScreenShake(5.0f, 0.15f);
            break;
        }
    }
    
    // Check if player is dead
    if (mHealth <= 0.0f)
    {
        GetGame()->GameOver();
    }
    
    // Keep player in world bounds
    Vector2 pos = GetPosition();
    float radius = 15.0f;
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
}

void Player::Heal(float amount)
{
    mHealth += amount;
    if (mHealth > mMaxHealth)
        mHealth = mMaxHealth;
}

void Player::AddExperience(float exp)
{
    mExperience += exp * mExperienceMultiplier;
    while (mExperience >= mExperienceToNextLevel)
    {
        mExperience -= mExperienceToNextLevel;
        mLevel++;
        mExperienceToNextLevel *= 1.5f; // Exponential growth
        
        // Screen shake on level up for epic feel!
        GetGame()->AddScreenShake(8.0f, 0.3f);
        
        GetGame()->ShowUpgradeMenu();
    }
}

void Player::UpdateAutoAttack(float deltaTime)
{
    mAttackCooldown -= deltaTime;
    
    if (mAttackCooldown <= 0.0f)
    {
        float baseCooldown = 0.5f / mAttackSpeedMultiplier;
        mAttackCooldown = baseCooldown;
        
        Vector2 playerPos = GetPosition();
        
        // Different attack patterns based on weapon modes
        if (mShotgunMode)
        {
            // Shotgun: Wide spread pattern
            int numProjectiles = mProjectileCount;
            float totalSpread = Math::Pi; // 180 degree spread
            for (int i = 0; i < numProjectiles; ++i)
            {
                float angle = (totalSpread / (numProjectiles - 1)) * i - totalSpread / 2.0f;
                Vector2 direction(Math::Cos(angle), Math::Sin(angle));
                Vector2 offset = direction * (15.0f + 10.0f);
                GetGame()->SpawnProjectile(playerPos + offset, direction, 1200.0f * mDamageMultiplier);
            }
        }
        else if (mSpiralMode)
        {
            // Spiral: Rotating spiral pattern
            static float spiralAngle = 0.0f;
            spiralAngle += 0.5f; // Rotate spiral
            int numProjectiles = mProjectileCount;
            for (int i = 0; i < numProjectiles; ++i)
            {
                float angle = spiralAngle + (Math::TwoPi / numProjectiles) * i;
                Vector2 direction(Math::Cos(angle), Math::Sin(angle));
                Vector2 offset = direction * (15.0f + 10.0f);
                GetGame()->SpawnProjectile(playerPos + offset, direction, 800.0f * mDamageMultiplier);
            }
        }
        else
        {
            // Default: Target nearest enemy or all directions
            Enemy* nearestEnemy = nullptr;
            float nearestDistance = 10000.0f;
            
            for (auto enemy : GetGame()->GetEnemies())
            {
                Vector2 enemyPos = enemy->GetPosition();
                Vector2 diff = enemyPos - playerPos;
                float distance = diff.Length();
                
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestEnemy = enemy;
                }
            }
            
            if (nearestEnemy)
            {
                Vector2 enemyPos = nearestEnemy->GetPosition();
                Vector2 direction = enemyPos - playerPos;
                direction.Normalize();
                
                float playerRadius = 15.0f + 10.0f;
                Vector2 spawnOffset = direction * playerRadius;
                
                int numProjectiles = mProjectileCount;
                float spreadAngle = 0.3f;
                
                for (int i = 0; i < numProjectiles; ++i)
                {
                    float angleOffset = ((i - numProjectiles / 2.0f) / numProjectiles) * spreadAngle;
                    float currentAngle = Math::Atan2(direction.y, direction.x) + angleOffset;
                    Vector2 projectileDirection(Math::Cos(currentAngle), Math::Sin(currentAngle));
                    
                    GetGame()->SpawnProjectile(playerPos + spawnOffset, projectileDirection, 800.0f * mDamageMultiplier);
                }
                
                // Reverse shot: Also shoot backwards
                if (mReverseShot)
                {
                    Vector2 reverseDir = direction * -1.0f;
                    Vector2 reverseOffset = spawnOffset * -1.0f;
                    GetGame()->SpawnProjectile(playerPos + reverseOffset, reverseDir, 800.0f * mDamageMultiplier);
                }
                
                // Rotate character to face nearest enemy
                float angle = Math::Atan2(direction.y, direction.x) + Math::PiOver2;
                SetRotation(angle);
            }
            else
            {
                // No enemies nearby, shoot in all directions
                float playerRadius = 15.0f + 10.0f;
                int numProjectiles = mProjectileCount;
                for (int i = 0; i < numProjectiles; ++i)
                {
                    float angle = (Math::TwoPi / numProjectiles) * i;
                    Vector2 direction(Math::Cos(angle), Math::Sin(angle));
                    Vector2 offset = direction * playerRadius;
                    
                    GetGame()->SpawnProjectile(playerPos + offset, direction, 800.0f * mDamageMultiplier);
                }
            }
        }
    }
}
