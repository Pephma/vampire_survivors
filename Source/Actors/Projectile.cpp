#include "Projectile.h"
#include "../Game.h"
#include "../Actors/Enemy.h"
#include "../Actors/Player.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimatorComponent.h"
#include "../Math.h"

Projectile::Projectile(class Game* game,
                       const Vector2& position,
                       const Vector2& direction,
                       float speed,
                       bool fromPlayer,
                       float damage,
                       int pierce,
                       bool homing,
                       bool explosive)
    : Actor(game)
    , mDirection(direction)
    , mSpeed(speed)
    , mLifetime(2.0f)
    , mFromPlayer(fromPlayer)
    , mDamage(damage)
    , mPierceRemaining(pierce)
    , mHoming(homing)
    , mHomingTurnRate(4.5f)  // radians per second - faster homing for better feel
    , mExplosive(explosive)
    , mExplosionRadius(80.0f)
{
    SetPosition(position);
    SetScale(Vector2(1.0f, -1.0f));
    
    // Normalize direction
    if (mDirection.LengthSq() > 0.0f)
    {
        mDirection.Normalize();
    }
    else
    {
        mDirection = Vector2(1.0f, 0.0f);  // Default direction
    }

    mAnimatorComponent = new AnimatorComponent(this,
        "../Assets/Sprites/Shot/Shot.png",
        "../Assets/Sprites/Shot/Shot.json",
        16,
        16
    );
    
    mAnimatorComponent->AddAnimation("shot", {0});
    mAnimatorComponent->SetAnimation("shot");
    mAnimatorComponent->SetAnimFPS(0.0f);

    float radius = 8.0f;
    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);

    mRigidBodyComponent->SetVelocity(mDirection * mSpeed);

    game->AddProjectile(this);
}

Projectile::~Projectile()
{
    GetGame()->RemoveProjectile(this);
}

void Projectile::UpdateHoming(float deltaTime)
{
    if (!mHoming || !mFromPlayer) return;
    
    // Find nearest enemy
    Enemy* nearestEnemy = nullptr;
    float nearestDistance = 10000.0f;
    
    for (auto* enemy : GetGame()->GetEnemies())
    {
        if (!enemy || enemy->GetState() != ActorState::Active) continue;
        
        // Skip enemies we've already hit
        if (mHitEnemies.find(enemy) != mHitEnemies.end()) continue;
        
        float dist = (enemy->GetPosition() - GetPosition()).LengthSq();
        if (dist < nearestDistance)
        {
            nearestDistance = dist;
            nearestEnemy = enemy;
        }
    }
    
    if (nearestEnemy && nearestDistance < 40000.0f)  // 200 units max range
    {
        Vector2 toEnemy = nearestEnemy->GetPosition() - GetPosition();
        toEnemy.Normalize();
        
        // Gradually turn toward enemy
        float currentAngle = Math::Atan2(mDirection.y, mDirection.x);
        float targetAngle = Math::Atan2(toEnemy.y, toEnemy.x);
        
        // Normalize angles
        float angleDiff = targetAngle - currentAngle;
        while (angleDiff > Math::Pi) angleDiff -= Math::TwoPi;
        while (angleDiff < -Math::Pi) angleDiff += Math::TwoPi;
        
        // Turn toward target
        float turnAmount = mHomingTurnRate * deltaTime;
        if (fabs(angleDiff) < turnAmount)
        {
            mDirection = toEnemy;
        }
        else
        {
            float newAngle = currentAngle + (angleDiff > 0 ? turnAmount : -turnAmount);
            mDirection = Vector2(Math::Cos(newAngle), Math::Sin(newAngle));
        }
        
        // Update velocity
        mRigidBodyComponent->SetVelocity(mDirection * mSpeed);
    }
}

void Projectile::CreateExplosion(const Vector2& position)
{
    if (!mExplosive) return;
    
    // Create explosion effect - only visual ring, no particles
    GetGame()->SpawnExplosionRing(position, mExplosionRadius);
    GetGame()->AddScreenShake(4.0f, 0.2f);  // Screen shake for explosions
    
    // Damage all enemies in radius (Boss inherits from Enemy, so bosses are included)
    int enemiesHit = 0;
    std::vector<class Enemy*> damagedEnemies;
    
    for (auto* enemy : GetGame()->GetEnemies())
    {
        if (!enemy || enemy->GetState() != ActorState::Active) continue;
        
        float dist = (enemy->GetPosition() - position).Length();
        if (dist < mExplosionRadius)
        {
            // Damage falls off with distance
            float damageMultiplier = 1.0f - (dist / mExplosionRadius) * 0.4f;  // 60% to 100% damage (less falloff)
            float actualDamage = enemy->TakeDamage(mDamage * damageMultiplier);
            
            // Show damage number
            if (actualDamage > 0.0f)
            {
                GetGame()->SpawnFloatingText(enemy->GetPosition(), std::to_string((int)actualDamage), Vector3(1.0f, 0.5f, 0.0f));
            }
            
            damagedEnemies.push_back(enemy);
            enemiesHit++;
        }
    }
    
    // Chain reaction - if explosive enemies die, they explode too!
    for (auto* enemy : damagedEnemies)
    {
        if (enemy && enemy->GetHealth() <= 0.0f)
        {
            // Check if this enemy explodes on death
            // This would require checking enemy type, but for now we'll trigger chain reactions
            // on any enemy death from explosion
            float chainRadius = mExplosionRadius * 0.6f; // Smaller chain radius
            Vector2 chainPos = enemy->GetPosition();
            
            // Small chain explosion - no particles
            
            // Damage nearby enemies in chain
            for (auto* otherEnemy : GetGame()->GetEnemies())
            {
                if (!otherEnemy || otherEnemy == enemy || otherEnemy->GetState() != ActorState::Active) continue;
                
                float chainDist = (otherEnemy->GetPosition() - chainPos).Length();
                if (chainDist < chainRadius)
                {
                    float chainDamage = mDamage * 0.5f; // Half damage for chain
                    otherEnemy->TakeDamage(chainDamage);
                }
            }
        }
    }
    
    // Visual feedback - no particles, just text if many enemies hit
    if (enemiesHit > 3)
    {
        GetGame()->SpawnFloatingText(position, "CHAIN!", Vector3(1.0f, 0.9f, 0.0f));
    }
}

void Projectile::OnUpdate(float deltaTime)
{
    // Update homing if enabled
    if (mHoming)
    {
        UpdateHoming(deltaTime);
    }
    
    // vida do projétil
    mLifetime -= deltaTime;
    if (mLifetime <= 0.0f)
    {
        GetGame()->RemoveProjectile(this);
        SetState(ActorState::Destroy);
        return;
    }

    // colisão
    if (mFromPlayer)
    {
        // projétil do jogador acerta inimigos
        for (auto* e : GetGame()->GetEnemies())
        {
            if (!e || e->GetState() != ActorState::Active) continue;
            
            // Skip enemies we've already hit (for pierce)
            if (mHitEnemies.find(e) != mHitEnemies.end()) continue;
            
            auto* enemyCol = e->GetComponent<CircleColliderComponent>();
            if (!enemyCol) continue;

            if (mCircleColliderComponent->Intersect(*enemyCol))
            {
                float actualDamage = e->TakeDamage(mDamage);
                
                // Lifesteal - heal player based on damage dealt
                auto* player = GetGame()->GetPlayer();
                if (player && player->HasLifesteal())
                {
                    float healAmount = actualDamage * player->GetLifestealPercent();
                    player->Heal(healAmount);
                }
                
                // Track hit enemy for pierce system
                mHitEnemies.insert(e);
                
                // Explosive projectiles removed - no explosion on hit
                
                // Check pierce
                if (mPierceRemaining > 0)
                {
                    mPierceRemaining--;
                    // Continue through enemy
                }
                else
                {
                    // No pierce left, destroy projectile
                    GetGame()->RemoveProjectile(this);
                    SetState(ActorState::Destroy);
                    return;
                }
            }
        }
    }
    else
    {
        // projétil inimigo acerta o jogador
        auto* p = GetGame()->GetPlayer();
        if (p && p->GetState() == ActorState::Active)
        {
            auto* playerCol = p->GetComponent<CircleColliderComponent>();
            if (playerCol && mCircleColliderComponent->Intersect(*playerCol))
            {
                p->TakeDamage(mDamage);

                GetGame()->RemoveProjectile(this);
                SetState(ActorState::Destroy);
                return;
            }
        }
    }

    // remover se sair dos limites do mundo
    Vector2 pos = GetPosition();
    if (pos.x < -100 || pos.x > Game::WORLD_WIDTH + 100 ||
        pos.y < -100 || pos.y > Game::WORLD_HEIGHT + 100)
    {
        GetGame()->RemoveProjectile(this);
        SetState(ActorState::Destroy);
        return;
    }
}
