#include "Boss.h"
#include "../Game.h"
#include "Player.h"
#include "DelayedExplosion.h" // Inclui o ator para o bombardeio
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/CircleColliderComponent.h"
#include "../Math.h"
#include "../Random.h"
#include <algorithm> // Necessário para std::find

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
    , mAttackCounter(0) // <-- ADICIONE A INICIALIZAÇÃO
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
            baseHealth = 15000.0f; // Vida aumentada
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
    auto iter = std::find(mComponents.begin(), mComponents.end(), mDrawComponent);
    if (iter != mComponents.end())
    {
        mComponents.erase(iter);
    }
    delete mDrawComponent;

    // 3. Recria os vértices com o RAIO CORRETO (baseRadius)
    std::vector<Vector2> vertices;
    const int numVertices = 20;
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * baseRadius, Math::Sin(angle) * baseRadius));
    }

    // 4. Cria o NOVO DrawComponent
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
            mStateTimer = 5.0f;
            break;
        case BossState::Cooldown:
            mStateTimer = 1.5f;
            break;

            // Ataques do Tank
        case BossState::BurstAttack:
            mStateTimer = 3.0f;
            mAttackSubTimer = 0.0f;
            break;
        case BossState::Dashing:
            mStateTimer = 0.7f;
            break;
        case BossState::Bombing:
            mStateTimer = 3.0f;
            mAttackSubTimer = 0.0f;
            break;

            // --- ATAQUES DO SPRAYER (MODIFICADOS) ---
        case BossState::Telegraphing: // Usado pelo Tank E Sprayer
            mStateTimer = 0.3f; // <-- TEMPO REDUZIDO (Menos tempo para fugir)
            break;
        case BossState::SpiralAttack:
            mStateTimer = 5.0f;
            mAttackSubTimer = 0.0f;
            break;
        case BossState::ConeAttack:
            mStateTimer = 2.5f; // <-- Duração aumentada (para mais leques)
            mAttackSubTimer = 0.0f; // Dispara o primeiro leque imediatamente
            break;
        case BossState::MineLayer:
            mStateTimer = 4.0f; // <-- Duração aumentada (para mais bombas)
            mAttackSubTimer = 0.0f;
            break;
        case BossState::FireBeam:
            mStateTimer = 0.5f;
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

        // --- Padrão do TANK (MODIFICADO) ---
        if (mKind == BossKind::Tank)
        {
            if (mBossState == BossState::Chasing)
            {
                // Escolhe o próximo ataque baseado no índice
                if (mAttackIndex == 0)      ChangeState(BossState::BurstAttack);
                else if (mAttackIndex == 1)
                {
                    mAttackCounter = 3; // <-- PREPARA 3 INVESTIDAS
                    ChangeState(BossState::Telegraphing); // Inicia a primeira investida
                }
                else if (mAttackIndex == 2) ChangeState(BossState::Bombing);

                // Avança o índice para o próximo ataque na rotação
                mAttackIndex = (mAttackIndex + 1) % 3; // Rotação 0, 1, 2
            }

            // --- Início do Loop da Investida ---
            else if (mBossState == BossState::Telegraphing)
            {
                ChangeState(BossState::Dashing); // Mira -> Corre
            }
            else if (mBossState == BossState::Dashing)
            {
                mAttackCounter--; // Investida executada
                if (mAttackCounter > 0)
                {
                    ChangeState(BossState::Telegraphing); // Ainda tem investidas? Mira de novo.
                }
                else
                {
                    ChangeState(BossState::Cooldown); // Acabou? Descansa.
                }
            }
            // --- Fim do Loop da Investida ---

            // Outros ataques (Burst, Bombing) vão para Cooldown
            else if (mBossState == BossState::BurstAttack ||
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
        // Padrão do SPRAYER (Sem mudança)
        else if (mKind == BossKind::Sprayer)
        {
            if (mBossState == BossState::Chasing)
            {
                // Escolhe o próximo ataque (agora 4 ataques)
                if (mAttackIndex == 0)      ChangeState(BossState::SpiralAttack);
                else if (mAttackIndex == 1) ChangeState(BossState::ConeAttack);
                else if (mAttackIndex == 2) ChangeState(BossState::MineLayer);
                else if (mAttackIndex == 3)
                {
                    mAttackCounter = 5; // Prepara 5 tiros de sniper
                    ChangeState(BossState::Telegraphing);
                }

                mAttackIndex = (mAttackIndex + 1) % 4; // Rotação 0, 1, 2, 3
            }
            else if (mBossState == BossState::Telegraphing)
            {
                ChangeState(BossState::FireBeam);
            }
            else if (mBossState == BossState::FireBeam)
            {
                mAttackCounter--;
                if (mAttackCounter > 0)
                {
                    ChangeState(BossState::Telegraphing);
                }
                else
                {
                    ChangeState(BossState::Cooldown);
                }
            }
            else if (mBossState == BossState::SpiralAttack ||
                     mBossState == BossState::ConeAttack ||
                     mBossState == BossState::MineLayer)
            {
                ChangeState(BossState::Cooldown);
            }
            else if (mBossState == BossState::Spawning || mBossState == BossState::Cooldown)
            {
                ChangeState(BossState::Chasing);
            }
        }
    }
}

// --- Implementações de cada estado ---

void Boss::UpdateSpawning(float deltaTime)
{
    mRigidBodyComponent->SetVelocity(Vector2(0.0f, 60.0f));
}

void Boss::UpdateChasing(float deltaTime)
{
    ChasePlayer(deltaTime);
}

void Boss::UpdateBurstAttack(float deltaTime)
{
    // ... (código original do BurstAttack) ...
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
    // ... (código original do SpiralAttack) ...
    ChasePlayer(deltaTime * 0.5f);
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
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mDrawComponent->SetColor(mBossColor); // Garante que a cor volte ao normal
}

void Boss::UpdateTelegraphing(float deltaTime)
{
    // ... (código original do Telegraphing) ...
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
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
    float blink = Math::Abs(Math::Sin(mStateTimer * 20.0f));
    mDrawComponent->SetColor(Vector3(1.0f, blink, blink));
}

void Boss::UpdateDashing(float deltaTime)
{
    // ... (código original do Dashing) ...
    mDrawComponent->SetColor(mBossColor);
    if (mRigidBodyComponent->GetVelocity().LengthSq() < 1.0f)
    {
        mRigidBodyComponent->SetVelocity(mTargetDirection * 800.0f);
    }
    mTargetDirection = Vector2::Zero;
}

void Boss::UpdateBombing(float deltaTime)
{
    // ... (código original do Bombing) ...
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mAttackSubTimer -= deltaTime;
    if (mAttackSubTimer <= 0.0f)
    {
        mAttackSubTimer = 0.7f;
        auto* player = GetGame()->GetPlayer();
        if (player)
        {
            Vector2 targetPos = player->GetPosition();
            targetPos.x += Random::GetFloatRange(-100.0f, 100.0f);
            targetPos.y += Random::GetFloatRange(-100.0f, 100.0f);
            new DelayedExplosion(GetGame(), targetPos, 1.5f, 75.0f);
        }
    }
}

// --- FUNÇÕES DE ATAQUE ADICIONADAS ---

void Boss::UpdateConeAttack(float deltaTime)
{
    // 1. Para de se mover
    mRigidBodyComponent->SetVelocity(Vector2::Zero);

    // 2. "Aquecimento"
    if (mAttackSubTimer > 0.0f) {
        mAttackSubTimer -= deltaTime;
        if (mAttackSubTimer <= 0.0f) {
            // --- HORA DE ATIRAR ---
            auto* player = GetGame()->GetPlayer();
            if (!player) return;

            // 1. Pega a direção principal para o jogador
            Vector2 dirToPlayer = player->GetPosition() - GetPosition();
            dirToPlayer.Normalize();
            float baseAngle = Math::Atan2(dirToPlayer.y, dirToPlayer.x);

            // 2. Define o "leque" (5 projéteis)
            const int numProjectiles = 5;
            const float spreadAngle = Math::Pi / 8.0f; // Ângulo total (22.5 graus)
            float startAngle = baseAngle - (spreadAngle / 2.0f);
            float angleStep = spreadAngle / (numProjectiles - 1);

            for (int i = 0; i < numProjectiles; i++) {
                float angle = startAngle + (i * angleStep);
                Vector2 dir(Math::Cos(angle), Math::Sin(angle));
                GetGame()->SpawnProjectile(GetPosition(), dir, 450.0f, false, 15.0f);
            }
            GetGame()->AddScreenShake(3.0f, 0.1f);
        }
    }
}

void Boss::UpdateMineLayer(float deltaTime)
{
    // 1. Persegue o jogador (MAIS RÁPIDO)
    auto* player = GetGame()->GetPlayer();
    if (player)
    {
        Vector2 playerPos = player->GetPosition();
        Vector2 enemyPos  = GetPosition();
        Vector2 dir = playerPos - enemyPos;

        float distance = dir.Length();
        if (distance > 0.01f)
        {
            dir.Normalize();
            // --- ALTERAÇÃO AQUI ---
            // Use a velocidade normal (mSpeed) multiplicada (ex: 1.5x ou 2x)
            mRigidBodyComponent->SetVelocity(dir * (mSpeed * 1.5f));
        }
    }

    // 2. Usa o sub-timer para soltar bombas (como antes)
    mAttackSubTimer -= deltaTime;
    if (mAttackSubTimer <= 0.0f) {
        mAttackSubTimer = 0.20f; // Solta bombas rápido

        // Cria uma bomba na posição ATUAL do chefe
        new DelayedExplosion(GetGame(), GetPosition(), 2.0f, 60.0f);
    }
}

void Boss::UpdateFireBeam(float deltaTime)
{
    // 1. Para de se mover e reseta a cor
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mDrawComponent->SetColor(mBossColor);

    // 2. Atira (só no primeiro frame deste estado)
    if (mTargetDirection.LengthSq() > 1e-4f) // Se a mira foi definida
    {
        GetGame()->SpawnProjectile(GetPosition(), mTargetDirection, 2000.0f, false, 50.0f); // Tiro rápido e forte!
        GetGame()->AddScreenShake(5.0f, 0.1f);
        mTargetDirection = Vector2::Zero; // Reseta a mira
    }
}