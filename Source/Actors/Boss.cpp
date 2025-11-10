#include "Boss.h"
#include "../Game.h"
#include "Player.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/CircleColliderComponent.h" // <-- Include Adicionado
#include "../Math.h"
#include "../Random.h"

// --- CONSTRUTOR MODIFICADO ---
Boss::Boss(Game* game, BossKind kind, int waveLevel)
    // 1. Chama o construtor do Enemy com 1.0 (que cria os componentes ruins)
    : Enemy(game, 1.0f, 1.0f, 1.0f)
    , mKind(kind) // Armazena o tipo
    , mStateTimer(0.0f)
    , mAttackSubTimer(0.0f)
    , mAttackAngle(0.0f)
{
    game->AddBoss(this);

    // --- ATRIBUTOS BASE DEFINIDOS PELO TIPO ---
    float baseHealth = 1.0f;
    float baseSpeed = 1.0f;
    float baseRadius = 1.0f;
    Vector3 bossColor; // Variável para guardar a cor

    switch (mKind)
    {
        case BossKind::Tank:
            baseHealth = 10000.0f;
            baseSpeed = 100.0f;
            baseRadius = 30.0f;
            bossColor = Vector3(0.5f, 0.1f, 0.9f); // Roxo
            break;

        case BossKind::Sprayer:
            baseHealth = 150.0f;
            baseSpeed = 70.0f;    // Mais rápido
            baseRadius = 25.0f;    // Um pouco menor
            bossColor = Vector3(0.1f, 0.9f, 0.5f); // Verde
            break;
    }

    // --- APLICA O ESCALAMENTO DA WAVE ---
    mHealth = baseHealth + (waveLevel * 50.0f);
    mMaxHealth = mHealth; // Importante para a barra de vida
    mSpeed = baseSpeed + (waveLevel * 2.0f);

    // --- CORREÇÃO DO CHEFE INVISÍVEL ---

    // 2. Deleta o DrawComponent antigo (de raio 1.0) que foi criado pelo Enemy()
    //    (Isso funciona porque movemos mDrawComponent para 'protected')
    delete mDrawComponent;

    // 3. Recria os vértices com o RAIO CORRETO (baseRadius)
    std::vector<Vector2> vertices;
    const int numVertices = 20; // Mais vértices para um chefe redondo
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * baseRadius, Math::Sin(angle) * baseRadius));
    }

    // 4. Cria o NOVO DrawComponent com os vértices e cor corretos
    mDrawComponent = new DrawComponent(this, vertices);
    mDrawComponent->SetColor(bossColor); // Aplica a cor do chefe
    mDrawComponent->SetFilled(true);

    // 5. Atualiza o RAIO DE COLISÃO (como já fazíamos)
    mCircleColliderComponent->SetRadius(baseRadius);
    // -------------------------------------------

    SetExperienceValue(100.0f * waveLevel);

    ChangeState(BossState::Spawning);
}
// ---------------------------------

Boss::~Boss()
{
    GetGame()->RemoveBoss(this);
}

void Boss::ChangeState(BossState newState)
{
    mBossState = newState;
    // Define a duração de cada estado
    switch (mBossState)
    {
        case BossState::Spawning:
            mStateTimer = 2.0f;
            break;
        case BossState::Chasing:
            mStateTimer = 6.0f;
            break;
        case BossState::BurstAttack: // Ataque do Tank
            mStateTimer = 3.0f;
            mAttackSubTimer = 0.0f;
            break;
        case BossState::SpiralAttack: // Ataque do Sprayer
            mStateTimer = 5.0f; // Dura mais tempo
            mAttackSubTimer = 0.0f;
            break;
        case BossState::Cooldown:
            mStateTimer = 1.5f;
            break;
    }
}

// --- OnUpdate MODIFICADO ---
void Boss::OnUpdate(float deltaTime)
{
    // 1. Lógica de Morte (essencial)
    if (mHealth <= 0.0f)
    {
        if (auto* player = GetGame()->GetPlayer())
        {
            player->AddExperience(mExperienceValue);
        }

        // Efeito de morte grande
        GetGame()->SpawnExplosionParticles(GetPosition(), Vector3(1.0f, 0.0f, 1.0f));
        GetGame()->SpawnExplosionParticles(GetPosition(), Vector3(1.0f, 1.0f, 1.0f));
        GetGame()->AddScreenShake(10.0f, 0.5f);

        SetState(ActorState::Destroy);
        return; // Para a execução
    }

    // 2. Lógica de Colisão com Projéteis
    // (Opcional, se o seu Projectile.cpp já aplica dano)
    // ...


    // 3. A Máquina de Estados (O Padrão de Ataque)
    mStateTimer -= deltaTime;

    switch (mBossState)
    {
        case BossState::Spawning:
            UpdateSpawning(deltaTime);
            break;
        case BossState::Chasing:
            UpdateChasing(deltaTime);
            break;
        case BossState::BurstAttack:
            UpdateBurstAttack(deltaTime);
            break;
        case BossState::SpiralAttack:
            UpdateSpiralAttack(deltaTime);
            break;
        case BossState::Cooldown:
            UpdateCooldown(deltaTime);
            break;
    }

    // 4. Mudar de estado se o tempo acabou
    if (mStateTimer <= 0.0f)
    {
        // --- LÓGICA DE MOVIMENTO/PADRÃO BASEADA NO TIPO ---

        // Padrão do TANK
        if (mKind == BossKind::Tank)
        {
            if (mBossState == BossState::Spawning)      ChangeState(BossState::Chasing);
            else if (mBossState == BossState::Chasing)  ChangeState(BossState::BurstAttack);
            else if (mBossState == BossState::BurstAttack) ChangeState(BossState::Cooldown);
            else if (mBossState == BossState::Cooldown) ChangeState(BossState::Chasing);
        }
        // Padrão do SPRAYER
        else if (mKind == BossKind::Sprayer)
        {
            if (mBossState == BossState::Spawning)      ChangeState(BossState::Chasing);
            else if (mBossState == BossState::Chasing)  ChangeState(BossState::SpiralAttack);
            else if (mBossState == BossState::SpiralAttack) ChangeState(BossState::Cooldown);
            else if (mBossState == BossState::Cooldown) ChangeState(BossState::Chasing);
        }
    }
}

// --- Implementações de cada estado ---

void Boss::UpdateSpawning(float deltaTime)
{
    // Movimento de entrada (igual para todos)
    mRigidBodyComponent->SetVelocity(Vector2(0.0f, 60.0f));
}

void Boss::UpdateChasing(float deltaTime)
{
    // Perseguição (igual para todos, mas a velocidade é diferente)
    ChasePlayer(deltaTime);
}

void Boss::UpdateBurstAttack(float deltaTime)
{
    // Ataque "Explosão" do Tank
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mAttackSubTimer -= deltaTime;
    if (mAttackSubTimer <= 0.0f)
    {
        mAttackSubTimer = 0.4f;
        const int numProjectiles = 8;
        for (int i = 0; i < numProjectiles; ++i)
        {
            float angle = (Math::TwoPi / numProjectiles) * i;
            Vector2 dir(Math::Cos(angle), Math::Sin(angle));
            GetGame()->SpawnProjectile(GetPosition(), dir, 300.0f, false, 20.0f);
        }
        GetGame()->AddScreenShake(2.0f, 0.1f);
    }
}

// --- NOVA FUNÇÃO DE ATAQUE ---
void Boss::UpdateSpiralAttack(float deltaTime)
{
    // Ataque "Espiral" do Sprayer
    // 1. Continua se movendo (padrão de movimento diferente)
    ChasePlayer(deltaTime * 0.5f); // Persegue LENTAMENTE enquanto atira

    // 2. Atira muito rápido em um ângulo que gira
    mAttackSubTimer -= deltaTime;
    if (mAttackSubTimer <= 0.0f)
    {
        mAttackSubTimer = 0.05f; // Atira muito rápido

        // Gira o ângulo
        mAttackAngle += 0.4f; // Radianos
        if (mAttackAngle > Math::TwoPi) mAttackAngle -= Math::TwoPi;

        Vector2 dir(Math::Cos(mAttackAngle), Math::Sin(mAttackAngle));
        GetGame()->SpawnProjectile(GetPosition(), dir, 400.0f, false, 15.0f); // Dano menor
    }
}

void Boss::UpdateCooldown(float deltaTime)
{
    // Fica parado (igual para todos)
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
}