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
    // círculo simples
    std::vector<Vector2> vertices;
    const int numVertices = 12;
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

    // timers de tiro
    mShootTimer = mShootEvery;

    game->AddEnemy(this);
}

Enemy::~Enemy()
{
    GetGame()->RemoveEnemy(this);
}

void Enemy::SetColor(const Vector3& c)
{
    if (mDrawComponent) mDrawComponent->SetColor(c);
}

void Enemy::OnUpdate(float deltaTime)
{
    // movimentação geral
    ChasePlayer(deltaTime);

    // tiro à distância (somente se marcado como ranged)
    TryShootAtPlayer(deltaTime);

    // colisão com projéteis do jogador
    auto player = GetGame()->GetPlayer();
    if (!player) return;

    for (auto projectile : GetGame()->GetProjectiles())
    {
        // projetil do jogador?
        // A lógica "quem atirou" está no Projectile (flag fromPlayer)
        auto circle = projectile->GetComponent<CircleColliderComponent>();
        if (!circle) continue;

        if (mCircleColliderComponent->Intersect(*circle))
        {
            // damage do projétil é aplicado dentro do Projectile no acerto do inimigo
            // Aqui só marcamos que levou dano (Projectile já chama TakeDamage no inimigo?).
            // Caso a sua implementação do Projectile não chame, mantemos o dano aqui:
            // (Se já estiver no Projectile, remover este bloco para evitar dano duplo)
        }
    }

    // morte
    if (mHealth <= 0.0f)
    {
        // Efeitos de morte
        if (mExplodesOnDeath)
        {
            // Gordo Explosivo: Lógica de explosão e círculo
            DoDeathExplosion();
        }
        else
        {
            // Outros inimigos: Partículas caindo
            // (Usando uma cor cinza/branca padrão para poeira)
            GetGame()->SpawnFallingParticles(GetPosition(), Vector3(0.8f, 0.8f, 0.8f));
        }

        // XP (acontece para todos)
        if (player)
        {
            player->AddExperience(mExperienceValue);
        }

        SetState(ActorState::Destroy);
    }
}

void Enemy::ChasePlayer(float deltaTime)
{
    auto player = GetGame()->GetPlayer();
    if (!player) return;

    Vector2 playerPos = player->GetPosition();
    Vector2 enemyPos  = GetPosition();
    Vector2 dir = playerPos - enemyPos;

    float distance = dir.Length();
    if (distance > 0.01f)
    {
        dir.Normalize();
        mRigidBodyComponent->SetVelocity(dir * mSpeed);
    }

    // dano por contato (DPS leve ao encostar)
    if (distance <= (mRadius + 15.0f))
    {
        player->TakeDamage(mDamage * 0.5f * deltaTime); // suaviza por deltaTime
        GetGame()->AddScreenShake(3.0f, 0.10f);
    }
}

void Enemy::TryShootAtPlayer(float deltaTime)
{
    if (!mIsRangedShooter) return;

    mShootTimer -= deltaTime;
    if (mShootTimer > 0.0f) return;

    auto* player = GetGame()->GetPlayer();
    if (!player) return;

    // direção para o player
    Vector2 from = GetPosition();
    Vector2 to   = player->GetPosition();
    Vector2 dir  = to - from;
    if (dir.LengthSq() < 1e-4f)
    {
        dir = Vector2(1.0f, 0.0f);
    }
    else
    {
        dir.Normalize();
    }

    // offset para não nascer em cima do inimigo
    const float muzzleOffset = mRadius + 6.0f;
    Vector2 spawnPos = from + dir * muzzleOffset;

    // dispara projétil "do inimigo": fromPlayer = false, dano = mDamage
    GetGame()->SpawnProjectile(spawnPos, dir, mProjectileSpeed, /*fromPlayer*/ false, /*damage*/ mDamage);

    // reseta timer
    mShootTimer = mShootEvery;
}

float Enemy::TakeDamage(float damage)
{
    float old = mHealth;
    mHealth -= damage;
    if (mHealth < 0.0f) mHealth = 0.0f;
    return old - mHealth;
}

void Enemy::DoDeathExplosion()
{
    auto* player = GetGame()->GetPlayer();
    Vector2 e = GetPosition(); // Posição do inimigo

    // Dano em área no jogador
    if (player)
    {
        Vector2 p = player->GetPosition();
        float dist = (p - e).Length();

        // Se o jogador estiver dentro do raio da explosão
        if (dist <= mExplosionRadius)
        {
            // Escala o dano com a distância (opcional)
            float falloff = 1.0f - Math::Clamp(dist / mExplosionRadius, 0.0f, 1.0f);
            player->TakeDamage(mExplosionDamage * (0.6f + 0.4f * falloff)); // Aplica dano no jogador
            GetGame()->AddScreenShake(6.0f, 0.2f); // Efeito de tremor de tela
        }
    }

    // --- Efeitos Visuais da Explosão ---

    // 1. Partículas da explosão (antiga SpawnDeathParticles, agora renomeada)
    GetGame()->SpawnExplosionParticles(e, Vector3(1.0f, 0.5f, 0.2f));

    // 2. NOVO: Anel de demarcação do raio
    GetGame()->SpawnExplosionRing(e, mExplosionRadius);
}

