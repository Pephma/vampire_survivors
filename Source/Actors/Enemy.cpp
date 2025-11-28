#include "Enemy.h"
#include "../Game.h"
#include "../Random.h"
#include "Projectile.h"
#include "Player.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimatorComponent.h"
#include "../Math.h"

Enemy::Enemy(class Game* game, EnemyKind kind, float radius, float speed, float health)
    : Actor(game)
    , mHealth(health)
    , mMaxHealth(health)
    , mSpeed(speed)
    , mRadius(radius)
    , mKind(kind)
    , mCurrentDirection(EnemyDirection::Front)
    , mWasCritKilled(false)
{
    // Configure sprite based on enemy type
    std::string spritePath;
    std::string jsonPath;
    int width, height;
    
    switch (kind)
    {
        case EnemyKind::Comum:
            spritePath = "../Assets/Sprites/Comum/Comum.png";
            jsonPath = "../Assets/Sprites/Comum/Comum.json";
            width = 20;
            height = 32;
            break;
        case EnemyKind::Corredor:
            spritePath = "../Assets/Sprites/Corredor/Corredor.png";
            jsonPath = "../Assets/Sprites/Corredor/Corredor.json";
            width = 24;
            height = 22;
            break;
        case EnemyKind::GordoExplosivo:
            spritePath = "../Assets/Sprites/Gordo/Gordo.png";
            jsonPath = "../Assets/Sprites/Gordo/Gordo.json";
            width = 32;
            height = 32;
            break;
        case EnemyKind::Atirador:
            spritePath = "../Assets/Sprites/Atirador/Atirador.png";
            jsonPath = "../Assets/Sprites/Atirador/Atirador.json";
            width = 24;
            height = 30;
            break;
        default:
            spritePath = "../Assets/Sprites/Comum/Comum.png";
            jsonPath = "../Assets/Sprites/Comum/Comum.json";
            width = 20;
            height = 32;
            break;
    }
    
    mAnimatorComponent = new AnimatorComponent(this, spritePath, jsonPath, width, height);
    
    // Setup animations based on enemy type
    if (kind == EnemyKind::Comum)
    {
        // Comum JSON indices: 0:Back, 1:Front, 2:Left2, 3:Left, 4:Back1, 5:Back2, 6:Front1, 7:Front2, 8:Left1
        mAnimatorComponent->AddAnimation("Back", {0, 4, 5});
        mAnimatorComponent->AddAnimation("Front", {1, 6, 7});
        mAnimatorComponent->AddAnimation("Left", {3, 8, 2});
        mAnimatorComponent->AddAnimation("Right", {3, 8, 2}); // Will be flipped
    }
    else if (kind == EnemyKind::Corredor)
    {
        // Corredor JSON indices: 0:Back, 1:Back1, 2:Back2, 3:Front1, 4:Front, 5:Front2, 6:Left1, 7:Left2, 8:Left
        mAnimatorComponent->AddAnimation("Back", {0, 1, 2});
        mAnimatorComponent->AddAnimation("Front", {4, 3, 5});
        mAnimatorComponent->AddAnimation("Left", {8, 6, 7});
        mAnimatorComponent->AddAnimation("Right", {8, 6, 7}); // Will be flipped
    }
    else if (kind == EnemyKind::GordoExplosivo)
    {
        // Gordo JSON indices: 0:Back, 1:Front, 2:Back1, 3:Left, 4:Back2, 5:Front1, 6:Front2, 7:Left1, 8:Left2
        mAnimatorComponent->AddAnimation("Back", {0, 2, 4});
        mAnimatorComponent->AddAnimation("Front", {1, 5, 6});
        mAnimatorComponent->AddAnimation("Left", {3, 7, 8});
        mAnimatorComponent->AddAnimation("Right", {3, 7, 8}); // Will be flipped
    }
    else if (kind == EnemyKind::Atirador)
    {
        // Atirador JSON indices: 0:Back1, 1:Front, 2:Back, 3:Left, 4:ShooterBack, 5:Back2, 6:ShooterFront, 7:ShooterLeft, 8:Front1, 9:Front2, 10:Left2, 11:Left1
        mAnimatorComponent->AddAnimation("Back", {2, 0, 5});
        mAnimatorComponent->AddAnimation("Front", {1, 8, 9});
        mAnimatorComponent->AddAnimation("Left", {3, 11, 10});
        mAnimatorComponent->AddAnimation("Right", {3, 11, 10}); // Will be flipped
    }
    
    mAnimatorComponent->SetAnimation("Front");
    mAnimatorComponent->SetAnimFPS(6.0f);

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

void Enemy::UpdateAnimation(const Vector2& directionToPlayer)
{
    // Determine which direction is dominant
    float absX = fabs(directionToPlayer.x);
    float absY = fabs(directionToPlayer.y);
    
    EnemyDirection newDirection = mCurrentDirection;
    
    if (absY > absX)
    {
        // Vertical movement is dominant
        if (directionToPlayer.y < 0.0f)
        {
            // Moving up (player is above)
            newDirection = EnemyDirection::Back;
            mAnimatorComponent->SetAnimation("Back");
            SetScale(Vector2(1.0f, -1.0f));
        }
        else
        {
            // Moving down (player is below)
            newDirection = EnemyDirection::Front;
            mAnimatorComponent->SetAnimation("Front");
            SetScale(Vector2(1.0f, -1.0f));
        }
    }
    else
    {
        // Horizontal movement is dominant
        if (directionToPlayer.x > 0.0f)
        {
            // Moving right (player is to the right)
            newDirection = EnemyDirection::Right;
            mAnimatorComponent->SetAnimation("Right");
            SetScale(Vector2(-1.0f, -1.0f));  // Flip horizontally for right
        }
        else
        {
            // Moving left (player is to the left)
            newDirection = EnemyDirection::Left;
            mAnimatorComponent->SetAnimation("Left");
            SetScale(Vector2(1.0f, -1.0f));
        }
    }
    
    mCurrentDirection = newDirection;
}

void Enemy::SetColor(const Vector3& c)
{

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
        UpdateAnimation(dir);
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

