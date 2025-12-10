#pragma once
#include "Enemy.h"
#include "../Game.h" // Precisa incluir para ter o BossKind

// Os "padrões de ataque" são definidos por estes estados
enum class BossState
{
    Spawning,
    Chasing,
    Cooldown,

    // Ataques do Tank
    BurstAttack,
    Telegraphing, // Reutilizado pelo Sprayer
    Dashing,
    Bombing,

    // --- NOVOS ESTADOS ADICIONADOS ---
    SpiralAttack, // Já existia
    ConeAttack,   // NOVO (Tiro em Leque)
    MineLayer,    // NOVO (Rastro de Bombas)
    FireBeam      // NOVO (Raio Telegrafado)
};

enum class BossDirection
{
    Front,
    Back,
    Left,
    Right
};

class Boss : public Enemy
{
public:
    Boss(class Game* game, BossKind kind, int waveLevel);
    ~Boss();
    void OnUpdate(float deltaTime) override;

private:
    void ChangeState(BossState newState);

    // Funções de estado
    void UpdateSpawning(float deltaTime);
    void UpdateChasing(float deltaTime);
    void UpdateCooldown(float deltaTime);

    // Funções de ataque do Tank
    void UpdateBurstAttack(float deltaTime);
    void UpdateTelegraphing(float deltaTime);
    void UpdateDashing(float deltaTime);
    void UpdateBombing(float deltaTime);

    // --- NOVAS FUNÇÕES DE ATAQUE ADICIONADAS ---
    void UpdateSpiralAttack(float deltaTime);
    void UpdateConeAttack(float deltaTime);
    void UpdateMineLayer(float deltaTime);
    void UpdateFireBeam(float deltaTime);

    void UpdateAnimation(Vector2& directionToPlayer);
    void ChasePlayer(float deltaTime);

    // Variáveis
    BossState mBossState;
    BossKind mKind;

    float mStateTimer;
    float mAttackSubTimer;
    float mAttackAngle;

    Vector2 mTargetDirection;
    int mAttackIndex;
    Vector3 mBossColor;

    int mAttackCounter;
    BossDirection mCurrentDirection;
};