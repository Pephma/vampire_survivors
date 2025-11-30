#include "Player.h"
#include "../Game.h"
#include "../Random.h"
#include "Enemy.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimatorComponent.h"
#include "../Components/ParticleSystemComponent.h"
#include "../Math.h"
#include <algorithm>

const float Player::DASH_COOLDOWN_TIME = 2.0f;
const float Player::DASH_DURATION_TIME = 0.15f;
const float Player::DASH_SPEED_MULTIPLIER = 3.5f;

Player::Player(class Game* game)
    : Actor(game)
    , mHealth(150.0f)  // Increased starting health for better survivability
    , mMaxHealth(150.0f)
    , mMoveSpeed(420.0f)  // Faster base movement for better feel
    , mAttackCooldown(0.0f)
    , mDamageMultiplier(1.0f)
    , mAttackSpeedMultiplier(1.0f)
    , mProjectileCount(10)  // Start with more projectiles for better feel
    , mHasHealthRegen(false)
    , mHealthRegenRate(5.0f)
    , mProjectilePierce(0)
    , mCritChance(0.0f)
    , mCritMultiplier(2.5f)  // Higher crit multiplier for more impact
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
    , mHasDash(false)
    , mDashCooldown(0.0f)
    , mDashDuration(0.0f)
    , mDashDirection(Vector2::Zero)
    , mExperience(0.0f)
    , mExperienceToNextLevel(150.0f)  // More difficult leveling (was 80.0f)
    , mLevel(1)
    , mPendingUpgrades(0)
    , mCurrentDirection(PlayerDirection::Front)
{
    SetPosition(Vector2(Game::WORLD_WIDTH / 2.0f, Game::WORLD_HEIGHT / 2.0f));
    SetScale(Vector2(1.0f, -1.0f)); // Flip Y to fix upside-down sprite
    
    // Create components
    mRigidBodyComponent = new RigidBodyComponent(this);
    mCircleColliderComponent = new CircleColliderComponent(this, 16.0f);
    mAnimatorComponent = new AnimatorComponent(this, 
        "../Assets/Sprites/Player/Player.png",
        "../Assets/Sprites/Player/Player.json",
        28,  // Average width
        32   // Average height
    );
    
    // Setup animations
    mAnimatorComponent->AddAnimation("Back", {0, 2, 4});
    mAnimatorComponent->AddAnimation("Front", {1, 5, 6});
    mAnimatorComponent->AddAnimation("Right", {7, 8, 3});
    mAnimatorComponent->AddAnimation("Left", {7, 8, 3});
    
    mAnimatorComponent->AddAnimation("IdleFront", {1});
    mAnimatorComponent->AddAnimation("IdleBack", {0});
    mAnimatorComponent->AddAnimation("IdleRight", {7});
    mAnimatorComponent->AddAnimation("IdleLeft", {7});
    
    mAnimatorComponent->SetAnimation("IdleFront");
    mAnimatorComponent->SetAnimFPS(8.0f);
    
    SetRotation(0.0f);
}

void Player::OnProcessInput(const Uint8* state)
{
    if (GetGame()->GetState() != MenuState::Playing)
    {
        return;
    }
    
    Vector2 moveDir(0.0f, 0.0f);
    
    if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP])
    {
        moveDir.y -= 1.0f;
    }
    if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN])
    {
        moveDir.y += 1.0f;
    }
    if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
    {
        moveDir.x -= 1.0f;
    }
    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
    {
        moveDir.x += 1.0f;
    }
    
    // Dash ability (SPACE key)
    if (mHasDash && mDashCooldown <= 0.0f && mDashDuration <= 0.0f)
    {
        if (state[SDL_SCANCODE_SPACE] && moveDir.LengthSq() > 0.0f)
        {
            moveDir.Normalize();
            mDashDirection = moveDir;
            mDashDuration = DASH_DURATION_TIME;
            mDashCooldown = DASH_COOLDOWN_TIME;
            GetGame()->AddScreenShake(2.0f, 0.1f);
            // Visual effect
            // No particles for dash
        }
    }
    
    if (moveDir.LengthSq() > 0.0f)
    {
        moveDir.Normalize();
        float currentSpeed = mMoveSpeed;
        
        // Apply dash speed multiplier
        if (mDashDuration > 0.0f)
        {
            currentSpeed *= DASH_SPEED_MULTIPLIER;
            mRigidBodyComponent->SetVelocity(mDashDirection * currentSpeed);
        }
        else
        {
            mRigidBodyComponent->SetVelocity(moveDir * currentSpeed);
        }
        UpdateAnimation(moveDir);
    }
    else if (mDashDuration <= 0.0f)
    {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        // Set idle animation when not moving
        if (mAnimatorComponent)
        {
            switch (mCurrentDirection)
            {
                case PlayerDirection::Back:
                    mAnimatorComponent->SetAnimation("IdleBack");
                    SetScale(Vector2(1.0f, -1.0f)); // Keep Y flipped
                    break;
                case PlayerDirection::Right:
                    mAnimatorComponent->SetAnimation("IdleRight");
                    SetScale(Vector2(1.0f, -1.0f)); // Keep Y flipped
                    break;
                case PlayerDirection::Left:
                    mAnimatorComponent->SetAnimation("IdleLeft");
                    SetScale(Vector2(-1.0f, -1.0f)); // Flip both X and Y
                    break;
                case PlayerDirection::Front:
                default:
                    mAnimatorComponent->SetAnimation("IdleFront");
                    SetScale(Vector2(1.0f, -1.0f)); // Keep Y flipped
                    break;
            }
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
    
    // Don't update if player is dead - stop all actions
    if (mHealth <= 0.0f)
    {
        // Stop movement and attacks
        if (mRigidBodyComponent)
        {
            mRigidBodyComponent->SetVelocity(Vector2::Zero);
        }
        return;  // Don't attack, don't move, don't do anything
    }
    
    // Update auto attack (shooting)
    UpdateAutoAttack(deltaTime);
    
    // Orbital weapons (if enabled)
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
                Vector2 tangentDir(-Math::Sin(angle), Math::Cos(angle)); // tangent
                GetGame()->SpawnProjectile(orbitalPos, tangentDir, 400.0f * mDamageMultiplier, /*fromPlayer*/ true, /*damage*/ 16.0f * mDamageMultiplier,
                    mProjectilePierce, mHomingProjectiles, mExplosiveProjectiles);
            }
            orbitalTimer = 0.0f;
        }
    }
    
    // Dash cooldown and duration
    if (mDashCooldown > 0.0f)
    {
        mDashCooldown -= deltaTime;
        if (mDashCooldown < 0.0f) mDashCooldown = 0.0f;
    }
    if (mDashDuration > 0.0f)
    {
        mDashDuration -= deltaTime;
        if (mDashDuration <= 0.0f)
        {
            mDashDuration = 0.0f;
            // Dash trail effect
            // No particles for dash end
        }
        else
        {
            // Update animation during dash to maintain correct direction
            UpdateAnimation(mDashDirection);
        }
    }
    else
    {
        // Update animation based on current velocity when not dashing
        if (mRigidBodyComponent)
        {
            Vector2 velocity = mRigidBodyComponent->GetVelocity();
            if (velocity.LengthSq() > 0.0f)
            {
                velocity.Normalize();
                UpdateAnimation(velocity);
            }
        }
    }
    
    // Health regen
    if (mHasHealthRegen && mHealth < mMaxHealth)
    {
        mHealth += mHealthRegenRate * deltaTime;
        if (mHealth > mMaxHealth) mHealth = mMaxHealth;
    }
    
    // Dano por contato com inimigos - reduced damage for better balance
    for (auto enemy : GetGame()->GetEnemies())
    {
        if (enemy && enemy->GetState() == ActorState::Active && mCircleColliderComponent)
        {
            auto* enemyCollider = enemy->GetComponent<CircleColliderComponent>();
            if (enemyCollider && mCircleColliderComponent->Intersect(*enemyCollider))
            {
                TakeDamage(6.0f * deltaTime);  // Reduced for better survivability
                GetGame()->AddScreenShake(5.0f, 0.15f);  // Better feedback
                break;
            }
        }
    }
    
    // Death check
    if (mHealth <= 0.0f)
    {
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
    if (mHealth < 0.0f) mHealth = 0.0f;
    
    if (mHealth <= 0.0f)
    {
        GetGame()->AddScreenShake(8.0f, 0.3f);
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
    
    // Check for level ups - but don't show menu multiple times
    bool shouldShowMenu = false;
    while (mExperience >= mExperienceToNextLevel)
    {
        mExperience -= mExperienceToNextLevel;
        mLevel++;
        mPendingUpgrades++;
        // Better XP scaling - much more difficult progression at each level
        mExperienceToNextLevel = 150.0f + (mLevel * mLevel * 15.0f);  // Exponential scaling - much harder at higher levels
        GetGame()->AddScreenShake(8.0f, 0.3f);
        
        // Mark that we should show menu, but only once
        if (!shouldShowMenu && GetGame()->GetState() == MenuState::Playing)
        {
            shouldShowMenu = true;
        }
    }
    
    // Show upgrade menu once after processing all level ups
    if (shouldShowMenu && GetGame()->GetState() == MenuState::Playing)
    {
        GetGame()->ShowUpgradeMenu();
    }
}

void Player::UpdateAnimation(const Vector2& moveDirection)
{
    if (!mAnimatorComponent) return;
    
    PlayerDirection newDirection = mCurrentDirection;
    
    if (moveDirection.LengthSq() > 0.0f)
    {
        // Determine which direction is dominant (similar to enemy logic)
        float absX = Math::Abs(moveDirection.x);
        float absY = Math::Abs(moveDirection.y);
        
        if (absY > absX)
        {
            // Vertical movement is dominant
            if (moveDirection.y < 0.0f)
            {
                // Moving up
                newDirection = PlayerDirection::Back;
                mAnimatorComponent->SetAnimation("Back");
                SetScale(Vector2(1.0f, -1.0f)); // Keep Y flipped
            }
            else
            {
                // Moving down
                newDirection = PlayerDirection::Front;
                mAnimatorComponent->SetAnimation("Front");
                SetScale(Vector2(1.0f, -1.0f)); // Keep Y flipped
            }
        }
        else
        {
            // Horizontal movement is dominant
            if (moveDirection.x > 0.0f)
            {
                // Moving right
                newDirection = PlayerDirection::Right;
                mAnimatorComponent->SetAnimation("Right");
                SetScale(Vector2(1.0f, -1.0f)); // Keep Y flipped, normal X
            }
            else
            {
                // Moving left
                newDirection = PlayerDirection::Left;
                mAnimatorComponent->SetAnimation("Right"); // Use right animation flipped
                SetScale(Vector2(-1.0f, -1.0f)); // Flip both X and Y for left
            }
        }
    }
    
    mCurrentDirection = newDirection;
}

void Player::UpdateAutoAttack(float deltaTime)
{
    mAttackCooldown -= deltaTime;
    
    float baseCooldown = 0.5f / mAttackSpeedMultiplier;
    
    if (mAttackCooldown <= 0.0f)
    {
        mAttackCooldown = baseCooldown;
        
        Vector2 playerPos = GetPosition();
        float baseSpeed = 750.0f;  // Even faster projectiles for spectacular feel
        float baseDamage = 25.0f * mDamageMultiplier;  // Better base damage for more satisfying kills
        
        // Apply crit chance
        bool isCrit = Random::GetFloatRange(0.0f, 1.0f) < mCritChance;
        if (isCrit)
        {
            baseDamage *= mCritMultiplier;
        }
        
        int numProjectiles = mProjectileCount;
        
        // Scale projectile count with level for spectacular progression
        int levelBonus = mLevel / 2;  // +1 projectile every 2 levels (faster scaling for epic feel)
        numProjectiles += levelBonus;
        
        // Scale damage with level for better progression
        float levelDamageBonus = 1.0f + (mLevel * 0.04f);  // +4% damage per level for better scaling
        baseDamage *= levelDamageBonus;
        
        if (mShotgunMode)
        {
            // Shotgun: fire at nearest enemy in a cone (no distance limit)
            Vector2 targetDirection(1.0f, 0.0f); // Default direction (right)
            
            // Find nearest enemy - no distance limit
            Enemy* nearestEnemy = nullptr;
            float nearestDistance = Math::Infinity; // Use infinity to find any enemy
            
            for (auto* enemy : GetGame()->GetEnemies())
            {
                if (!enemy || enemy->GetState() != ActorState::Active) continue;
                
                Vector2 toEnemy = enemy->GetPosition() - playerPos;
                float dist = toEnemy.LengthSq();
                if (dist < nearestDistance)
                {
                    nearestDistance = dist;
                    nearestEnemy = enemy;
                }
            }
            
            // If we found an enemy, aim at it (no distance check)
            if (nearestEnemy)
            {
                targetDirection = nearestEnemy->GetPosition() - playerPos;
                if (targetDirection.LengthSq() > 0.0001f) // Avoid division by zero
                {
                    targetDirection.Normalize();
                }
                else
                {
                    targetDirection = Vector2(1.0f, 0.0f); // Default if enemy is on top of player
                }
            }
            
            // Calculate base angle from target direction
            float baseAngle = Math::Atan2(targetDirection.y, targetDirection.x);
            
            // Shotgun: fire in a cone centered on nearest enemy
            float coneAngle = Math::Pi / 4.0f;  // 45 degrees spread
            for (int i = 0; i < numProjectiles; ++i)
            {
                float angle = baseAngle - coneAngle / 2.0f + (coneAngle / (numProjectiles - 1)) * i;
                Vector2 dir(Math::Cos(angle), Math::Sin(angle));
                GetGame()->SpawnProjectile(playerPos, dir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage,
                    mProjectilePierce, mHomingProjectiles, mExplosiveProjectiles);
            }
        }
        else if (mSpiralMode)
        {
            // Spiral: projectiles spiral outward
            static float spiralAngle = 0.0f;
            spiralAngle += Math::TwoPi / numProjectiles;
            for (int i = 0; i < numProjectiles; ++i)
            {
                float angle = spiralAngle + (Math::TwoPi / numProjectiles) * i;
                Vector2 dir(Math::Cos(angle), Math::Sin(angle));
                GetGame()->SpawnProjectile(playerPos, dir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage,
                    mProjectilePierce, mHomingProjectiles, mExplosiveProjectiles);
            }
        }
        else if (mOrbitalWeapons)
        {
            // Orbital: projectiles orbit around player
            mOrbitalAngle += deltaTime * 3.0f;
            for (int i = 0; i < mOrbitalCount; ++i)
            {
                float angle = mOrbitalAngle + (Math::TwoPi / mOrbitalCount) * i;
                float radius = 50.0f;
                Vector2 offset(Math::Cos(angle) * radius, Math::Sin(angle) * radius);
                Vector2 projPos = playerPos + offset;
                Vector2 dir(Math::Cos(angle + Math::Pi / 2.0f), Math::Sin(angle + Math::Pi / 2.0f));
                GetGame()->SpawnProjectile(projPos, dir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage,
                    mProjectilePierce, mHomingProjectiles, mExplosiveProjectiles);
            }
        }
        else
        {
            // Normal: fire in all directions
            float playerRadius = 20.0f;
            for (int i = 0; i < numProjectiles; ++i)
            {
                float angle = (Math::TwoPi / numProjectiles) * i;
                Vector2 dir(Math::Cos(angle), Math::Sin(angle));
                dir.Normalize();
                
                Vector2 offset = dir * playerRadius;
                GetGame()->SpawnProjectile(playerPos + offset, dir, baseSpeed, /*fromPlayer*/ true, /*damage*/ baseDamage,
                    mProjectilePierce, mHomingProjectiles, mExplosiveProjectiles);
            }
        }
    }
}
