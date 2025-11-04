#include "Enemy.h"
#include "../Game.h"
#include "../Random.h"
#include "Projectile.h"
#include "Player.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Math.h"

Enemy::Enemy(class Game* game, float radius, float speed, float health)
    : Actor(game)
    , mHealth(health)
    , mMaxHealth(health)
    , mSpeed(speed)
    , mRadius(radius)
    , mWasCritKilled(false)
{
    // Create circular enemy shape
    std::vector<Vector2> vertices;
    int numVertices = 12;
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * radius, Math::Sin(angle) * radius));
    }
    
    mDrawComponent = new DrawComponent(this, vertices);
    // More vibrant enemy colors - dark red with glow
    Vector3 enemyColor = Vector3(0.9f, 0.1f, 0.1f); // Bright red
    mDrawComponent->SetColor(enemyColor);
    mDrawComponent->SetFilled(true); // Filled for better visibility
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);
    
    game->AddEnemy(this);
}

Enemy::~Enemy()
{
    GetGame()->RemoveEnemy(this);
}

void Enemy::OnUpdate(float deltaTime)
{
    ChasePlayer(deltaTime);
    
    // Check collision with player projectiles
    auto player = GetGame()->GetPlayer();
    if (!player)
        return;
    
    for (auto projectile : GetGame()->GetProjectiles())
    {
        if (mCircleColliderComponent->Intersect(*projectile->GetComponent<CircleColliderComponent>()))
        {
            // Calculate base damage
            float baseDamage = 20.0f * player->GetDamageMultiplier();
            
            // Check for crit
            bool isCrit = false;
            if (player->GetCritChance() > 0.0f)
            {
                float critRoll = Random::GetFloat();
                if (critRoll < player->GetCritChance())
                {
                    isCrit = true;
                    baseDamage *= player->GetCritMultiplier();
                }
            }
            
            // Apply damage
            float damageDealt = TakeDamage(baseDamage);
            
            // Apply lifesteal
            if (player->HasLifesteal() && damageDealt > 0.0f)
            {
                float healAmount = damageDealt * player->GetLifestealPercent();
                player->Heal(healAmount);
            }
            
            // Handle pierce - if pierce is enabled, allow projectile to continue
            int pierce = player->GetProjectilePierce();
            if (pierce <= 0)
            {
                // No pierce - destroy projectile on hit
                projectile->SetState(ActorState::Destroy);
            }
            // If pierce > 0, projectile continues (simplified - ideally each projectile tracks its own hits)
            
            // Store crit status for death effect
            mWasCritKilled = isCrit;
            
            break;
        }
    }
    
    // Check if dead
    if (mHealth <= 0.0f)
    {
        // Screen shake on kill
        if (mWasCritKilled)
        {
            // Bigger screen shake for crit kills
            GetGame()->AddScreenShake(4.0f, 0.15f);
        }
        else
        {
            GetGame()->AddScreenShake(2.0f, 0.1f);
        }
        
        // Give player experience
        if (player)
        {
            player->AddExperience(10.0f);
        }
        GetGame()->RemoveEnemy(this);
        SetState(ActorState::Destroy);
    }
}

float Enemy::TakeDamage(float damage)
{
    float oldHealth = mHealth;
    mHealth -= damage;
    if (mHealth < 0.0f)
        mHealth = 0.0f;
    
    // Return actual damage dealt
    return oldHealth - mHealth;
}

void Enemy::ChasePlayer(float deltaTime)
{
    auto player = GetGame()->GetPlayer();
    if (!player)
        return;
    
    Vector2 playerPos = player->GetPosition();
    Vector2 enemyPos = GetPosition();
    Vector2 direction = playerPos - enemyPos;
    
    float distance = direction.Length();
    if (distance > 0.01f)
    {
        direction.Normalize();
        mRigidBodyComponent->SetVelocity(direction * mSpeed);
    }
}

