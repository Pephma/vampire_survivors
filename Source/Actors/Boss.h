// Em Actors/Boss.h

#pragma once
#include "Enemy.h"
#include "../Game.h" // Precisa incluir para ter o BossKind

// Os "padrões de ataque" são definidos por estes estados
enum class BossState
{
    Spawning,
    Chasing,
    BurstAttack,  // Ataque do "Tank"
    SpiralAttack, // --- NOVO ESTADO DE ATAQUE ---
    Cooldown
};

class Boss : public Enemy
{
public:
    // --- CONSTRUTOR MODIFICADO ---
    Boss(class Game* game, BossKind kind, int waveLevel);
    // -----------------------------

    ~Boss();

    void OnUpdate(float deltaTime) override;

private:
    void ChangeState(BossState newState);

    // Funções de update para cada estado (cada "padrão")
    void UpdateSpawning(float deltaTime);
    void UpdateChasing(float deltaTime);
    void UpdateBurstAttack(float deltaTime);
    void UpdateSpiralAttack(float deltaTime); // --- NOVA FUNÇÃO DE ATAQUE ---
    void UpdateCooldown(float deltaTime);

    BossState mBossState;
    BossKind mKind; // <-- Armazena o tipo do chefe

    float mStateTimer;
    float mAttackSubTimer;
    float mAttackAngle; // <-- Usado para o ataque espiral
};