#include "Boss.h"
#include "../Game.h"
#include "Player.h"
#include "DelayedExplosion.h" // Inclui o ator para o bombardeio
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/CircleColliderComponent.h"
#include "../Math.h"
#include "../Random.h"
#include <algorithm>

// --- CONSTRUTOR MODIFICADO ---
Boss::Boss(Game* game, BossKind kind, int waveLevel)
    // 1. Chama o construtor do Enemy com 1.0 (que cria os componentes ruins)
    : Enemy(game, 1.0f, 1.0f, 1.0f)
    , mKind(kind) // Armazena o tipo
    , mStateTimer(0.0f)
    , mAttackSubTimer(0.0f)
    , mAttackAngle(0.0f)
    , mTargetDirection(Vector2::Zero) // Inicializa a direção do dash
    , mAttackIndex(0)                 // Inicializa o índice de rotação de ataques
    , mBossColor(Vector3::One)        // Inicializa a cor base
{
    game->AddBoss(this);
    // (Não removemos o inimigo daqui, pois o ~Enemy já faz isso na morte)

    // --- ATRIBUTOS BASE DEFINIDOS PELO TIPO ---
    float baseHealth = 1.0f;
    float baseSpeed = 1.0f;
    float baseRadius = 1.0f;

    switch (mKind)
    {
        case BossKind::Tank:
            baseHealth = 10000.0f; // Sua vida alta
            baseSpeed = 100.0f;    // Sua velocidade
            baseRadius = 30.0f;
            mBossColor = Vector3(0.5f, 0.1f, 0.9f); // Roxo
            break;

        case BossKind::Sprayer:
            baseHealth = 15000.0f;
            baseSpeed = 70.0f;    // Mais rápido
            baseRadius = 25.0f;    // Um pouco menor
            mBossColor = Vector3(0.1f, 0.9f, 0.5f); // Verde
            break;
    }

    // --- APLICA O ESCALAMENTO DA WAVE ---
    mHealth = baseHealth + (waveLevel * 50.0f);
    mMaxHealth = mHealth; // Importante para a barra de vida
    mSpeed = baseSpeed + (waveLevel * 2.0f);

    // --- CORREÇÃO DO CHEFE INVISÍVEL E TRAVAMENTO (DOUBLE DELETE) ---

    // 2. Deleta o DrawComponent antigo (de raio 1.0) que foi criado pelo Enemy()

    // --- ADICIONE ESTE BLOCO PRIMEIRO ---
    // Encontra e remove o ponteiro do mDrawComponent da lista mComponents do Actor
    // (mComponents é 'protected' em Actor, então 'Boss' pode acessá-lo)
    // É crucial fazer isso ANTES de 'delete' para evitar um "double delete"
    auto iter = std::find(mComponents.begin(), mComponents.end(), mDrawComponent);
    if (iter != mComponents.end())
    {
        mComponents.erase(iter);
    }
    // ------------------------------------

    delete mDrawComponent; // <-- Agora é seguro deletar o componente antigo

    // 3. Recria os vértices com o RAIO CORRETO (baseRadius)
    std::vector<Vector2> vertices;
    const int numVertices = 20; // Mais vértices para um chefe redondo
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * baseRadius, Math::Sin(angle) * baseRadius));
    }

    // 4. Cria o NOVO DrawComponent (que se adicionará automaticamente a mComponents)
    mDrawComponent = new DrawComponent(this, vertices);
    mDrawComponent->SetColor(mBossColor); // Aplica a cor do chefe
    mDrawComponent->SetFilled(true);

    // 5. Atualiza o RAIO DE COLISÃO
    mCircleColliderComponent->SetRadius(baseRadius);
    // -------------------------------------------

    SetExperienceValue(100.0f * waveLevel);

    ChangeState(BossState::Spawning);
}

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
            mStateTimer = 5.0f; // Persegue por 5 segundos
            break;
        case BossState::Cooldown:
            mStateTimer = 1.5f;
            break;

        // Ataques do Tank
        case BossState::BurstAttack:
            mStateTimer = 3.0f;
            mAttackSubTimer = 0.0f;
            break;
        case BossState::Telegraphing:
            mStateTimer = 1.2f; // Tempo para o jogador reagir ao dash
            break;
        case BossState::Dashing:
            mStateTimer = 0.7f; // Duração da investida
            break;
        case BossState::Bombing:
            mStateTimer = 3.0f; // Tempo que ele fica parado bombardeando
            mAttackSubTimer = 0.0f;
            break;

        // Ataque do Sprayer
        case BossState::SpiralAttack:
            mStateTimer = 5.0f;
            mAttackSubTimer = 0.0f;
            break;
    }
}

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
    // ...


    // 3. A Máquina de Estados (O Padrão de Ataque)
    mStateTimer -= deltaTime;

    switch (mBossState)
    {
        case BossState::Spawning: UpdateSpawning(deltaTime); break;
        case BossState::Chasing: UpdateChasing(deltaTime); break;
        case BossState::Cooldown: UpdateCooldown(deltaTime); break;

        // Ataques
        case BossState::BurstAttack: UpdateBurstAttack(deltaTime); break;
        case BossState::SpiralAttack: UpdateSpiralAttack(deltaTime); break;
        case BossState::Telegraphing: UpdateTelegraphing(deltaTime); break;
        case BossState::Dashing: UpdateDashing(deltaTime); break;
        case BossState::Bombing: UpdateBombing(deltaTime); break;
    }

    // 4. Mudar de estado se o tempo acabou
    if (mStateTimer <= 0.0f)
    {
        // --- LÓGICA DE MOVIMENTO/PADRÃO BASEADA NO TIPO ---

        // Padrão do TANK
        if (mKind == BossKind::Tank)
        {
            if (mBossState == BossState::Chasing)
            {
                // Escolhe o próximo ataque baseado no índice
                if (mAttackIndex == 0)      ChangeState(BossState::BurstAttack);
                else if (mAttackIndex == 1) ChangeState(BossState::Telegraphing); // Inicia a sequência do Dash
                else if (mAttackIndex == 2) ChangeState(BossState::Bombing);

                // Avança o índice para o próximo ataque na rotação
                mAttackIndex = (mAttackIndex + 1) % 3; // Rotação 0, 1, 2, 0, 1, ...
            }
            // Sequência do Dash
            else if (mBossState == BossState::Telegraphing)
            {
                ChangeState(BossState::Dashing);
            }
            // Ataques que vão para Cooldown
            else if (mBossState == BossState::BurstAttack ||
                     mBossState == BossState::Dashing ||
                     mBossState == BossState::Bombing)
            {
                ChangeState(BossState::Cooldown);
            }
            // Volta a perseguir
            else if (mBossState == BossState::Spawning || mBossState == BossState::Cooldown)
            {
                ChangeState(BossState::Chasing);
            }
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

void Boss::UpdateSpiralAttack(float deltaTime)
{
    // Ataque "Espiral" do Sprayer
    ChasePlayer(deltaTime * 0.5f); // Persegue LENTAMENTE enquanto atira
    mAttackSubTimer -= deltaTime;
    if (mAttackSubTimer <= 0.0f)
    {
        mAttackSubTimer = 0.05f;
        mAttackAngle += 0.4f;
        if (mAttackAngle > Math::TwoPi) mAttackAngle -= Math::TwoPi;
        Vector2 dir(Math::Cos(mAttackAngle), Math::Sin(mAttackAngle));
        GetGame()->SpawnProjectile(GetPosition(), dir, 400.0f, false, 15.0f);
    }
}

void Boss::UpdateCooldown(float deltaTime)
{
    // Fica parado (igual para todos)
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mDrawComponent->SetColor(mBossColor); // Garante que a cor volte ao normal
}

// --- FUNÇÕES DE ATAQUE ADICIONADAS ---

void Boss::UpdateTelegraphing(float deltaTime)
{
    // 1. Para de se mover
    mRigidBodyComponent->SetVelocity(Vector2::Zero);

    // 2. Armazena a direção do jogador (só no primeiro frame deste estado)
    if (mTargetDirection.LengthSq() < 1e-4f)
    {
        auto* player = GetGame()->GetPlayer();
        if (player)
        {
            mTargetDirection = player->GetPosition() - GetPosition();
            mTargetDirection.Normalize();
        }
        else
        {
            mTargetDirection = Vector2(0.0f, 1.0f); // Padrão
        }
    }

    // 3. Efeito visual (piscar)
    float blink = Math::Abs(Math::Sin(mStateTimer * 20.0f));
    mDrawComponent->SetColor(Vector3(1.0f, blink, blink)); // Pisca em branco/vermelho
}

void Boss::UpdateDashing(float deltaTime)
{
    // 1. Reseta a cor (caso tenha piscado)
    mDrawComponent->SetColor(mBossColor);

    // 2. Define a velocidade alta na direção armazenada
    if (mRigidBodyComponent->GetVelocity().LengthSq() < 1.0f)
    {
        mRigidBodyComponent->SetVelocity(mTargetDirection * 800.0f); // Velocidade do Dash
    }

    // 3. Reseta a direção para o próximo ataque
    mTargetDirection = Vector2::Zero;
}

void Boss::UpdateBombing(float deltaTime)
{
    // 1. Fica parado
    mRigidBodyComponent->SetVelocity(Vector2::Zero);

    // 2. Usa o sub-timer para soltar bombas
    mAttackSubTimer -= deltaTime;
    if (mAttackSubTimer <= 0.0f)
    {
        mAttackSubTimer = 0.7f; // Solta uma bomba a cada 0.7s

        auto* player = GetGame()->GetPlayer();
        if (player)
        {
            // Cria uma bomba na posição do jogador
            Vector2 targetPos = player->GetPosition();

            // Adiciona um pequeno desvio aleatório
            targetPos.x += Random::GetFloatRange(-100.0f, 100.0f);
            targetPos.y += Random::GetFloatRange(-100.0f, 100.0f);

            // Cria o ator 'DelayedExplosion'
            // (game, posição, delay, raio)
            new DelayedExplosion(GetGame(), targetPos, 1.5f, 75.0f);
        }
    }
}