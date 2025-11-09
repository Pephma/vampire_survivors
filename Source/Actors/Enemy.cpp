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
    , mExplodesOnDeath(false)
    , mRangedShooter(false)
    , mShootEvery(0.0f)
    , mShootTimer(0.0f)
{
    // círculo
    std::vector<Vector2> vertices;
    int numVertices = 12;
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * radius, Math::Sin(angle) * radius));
    }

    mDrawComponent = new DrawComponent(this, vertices);
    mDrawComponent->SetColor(Vector3(0.9f, 0.1f, 0.1f));
    mDrawComponent->SetFilled(true);

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

    // Tiros de inimigo (Atirador)
    if (mRangedShooter && mShootEvery > 0.0f)
    {
        mShootTimer += deltaTime;
        if (mShootTimer >= mShootEvery)
        {
            mShootTimer = 0.0f;
            auto* player = GetGame()->GetPlayer();
            if (player)
            {
                Vector2 from = GetPosition();
                Vector2 to   = player->GetPosition();
                Vector2 dir  = to - from;
                if (dir.LengthSq() > 1e-4f)
                {
                    dir.Normalize();
                    // Nota: usa o Projectile existente (causa dano em inimigos). Para separar,
                    // crie um EnemyProjectile específico. Para protótipo, mantém assim.
                    GetGame()->SpawnProjectile(from, dir, 500.0f);
                }
            }
        }
    }

    // Colisão com projéteis do jogador
    auto player = GetGame()->GetPlayer();
    if (!player) return;

    for (auto projectile : GetGame()->GetProjectiles())
    {
        if (mCircleColliderComponent->Intersect(*projectile->GetComponent<CircleColliderComponent>()))
        {
            float baseDamage = 20.0f * player->GetDamageMultiplier();

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

            float damageDealt = TakeDamage(baseDamage);

            if (player->HasLifesteal() && damageDealt > 0.0f)
            {
                float healAmount = damageDealt * player->GetLifestealPercent();
                player->Heal(healAmount);
            }

            int pierce = player->GetProjectilePierce();
            if (pierce <= 0)
            {
                projectile->SetState(ActorState::Destroy);
            }

            mWasCritKilled = isCrit;
            break;
        }
    }

    // Morte
    if (mHealth <= 0.0f)
    {
        if (mWasCritKilled)
            GetGame()->AddScreenShake(4.0f, 0.15f);
        else
            GetGame()->AddScreenShake(2.0f, 0.1f);

        // Explosão em área (gordo/explosivo)
        if (mExplodesOnDeath)
        {
            auto* p = GetGame()->GetPlayer();
            if (p)
            {
                float aoeR = mRadius + 80.0f;
                if ((p->GetPosition() - GetPosition()).Length() <= aoeR)
                    p->TakeDamage(25.0f);
            }
        }

        if (player) player->AddExperience(10.0f);

        GetGame()->RemoveEnemy(this);
        SetState(ActorState::Destroy);
    }
}

float Enemy::TakeDamage(float damage)
{
    float old = mHealth;
    mHealth -= damage;
    if (mHealth < 0.0f) mHealth = 0.0f;
    return old - mHealth;
}

void Enemy::ChasePlayer(float deltaTime)
{
    auto player = GetGame()->GetPlayer();
    if (!player) return;

    Vector2 playerPos = player->GetPosition();
    Vector2 enemyPos  = GetPosition();
    Vector2 direction = playerPos - enemyPos;

    float distance = direction.Length();
    if (distance > 0.01f)
    {
        direction.Normalize();
        mRigidBodyComponent->SetVelocity(direction * mSpeed);
    }
}

void Enemy::SetColor(const Vector3& color)
{
    if (mDrawComponent) mDrawComponent->SetColor(color);
}
