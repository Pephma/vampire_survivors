// Em Actors/Boss.h

#pragma once
#include "Enemy.h"
#include "../Game.h" // Precisa incluir para ter o BossKind

// Os "padrões de ataque" são definidos por estes estados
enum class BossState
{
    Spawning,
    Chasing,
    BurstAttack,  // Ataque do "Tank" (Já existe)
    SpiralAttack, // Ataque do "Sprayer" (Já existe)
    Cooldown,

    // --- NOVOS ESTADOS ADICIONADOS ---
    Telegraphing, // Mirando a investida (Dash)
    Dashing,      // Executando a investida
    Bombing       // Executando o bombardeio
};

class Boss : public Enemy
{
public:
    Boss(class Game* game, BossKind kind, int waveLevel);
    ~Boss();
    void OnUpdate(float deltaTime) override;

private:
    void ChangeState(BossState newState);

    // Funções de update para cada estado (cada "padrão")
    void UpdateSpawning(float deltaTime);
    void UpdateChasing(float deltaTime);
    void UpdateBurstAttack(float deltaTime);
    void UpdateSpiralAttack(float deltaTime);
    void UpdateCooldown(float deltaTime);

    // --- NOVAS FUNÇÕES DE ATAQUE ADICIONADAS ---
    void UpdateTelegraphing(float deltaTime);
    void UpdateDashing(float deltaTime);
    void UpdateBombing(float deltaTime);

    BossState mBossState;
    BossKind mKind;

    float mStateTimer;
    float mAttackSubTimer;
    float mAttackAngle;

    // --- NOVAS VARIÁVEIS ADICIONADAS ---
    Vector2 mTargetDirection; // Para onde o Dash vai
    int mAttackIndex;         // Para controlar a rotação de ataques

    Vector3 mBossColor;
};