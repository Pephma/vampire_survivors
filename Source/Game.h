#pragma once
#include <SDL.h>
#include <vector>
#include "Renderer/Renderer.h"
#include "Audio/AudioSystem.h"

// ============================================
//  NOVOS ENUMS E ESTRUTURAS DE SPAWN
// ============================================
enum class EnemyKind { Comum, Corredor, GordoExplosivo, Atirador };


enum class BossKind
{
    Tank,       // O chefe (Lento, vida alta, ataque Burst)
    Sprayer     // Novo chefe (Rápido, vida média, ataque Espiral)
};

struct SpawnRule {
    EnemyKind kind;
    int       minWave;
    float     start;
    float     end;
    float     every;
    int       count;
};

struct TimedHorde {
    float atTime;
    EnemyKind kind;
    int count;
    int minWave;
    bool fired = false;
};
// ============================================

enum class MenuState
{
    MainMenu,
    Playing,
    Paused,
    UpgradeMenu,
    GameOver
};

class Game
{
public:
    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // --------------------------------
    // NOVAS FUNÇÕES PARA O SPAWN SYSTEM
    // --------------------------------
    void InitSpawnRules();
    void SpawnEnemyOfKind(EnemyKind kind, int count);
    void SpawnHorde(EnemyKind kind, int count);
    // --------------------------------

    // Existing declarations ...
    void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);

    class Renderer* GetRenderer() { return mRenderer; }
    AudioSystem* GetAudioSystem() { return mAudioSystem; }

    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;
    static const int WORLD_WIDTH = 4000;
    static const int WORLD_HEIGHT = 4000;

    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    class Player* GetPlayer() const { return mPlayer; }

    Vector2 GetCameraPosition() const { return mCameraPosition; }
    void UpdateCamera(float deltaTime);
    void AddScreenShake(float intensity, float duration = 0.2f);
    void SpawnExplosionParticles(const Vector2& position, const Vector3& color);

    void AddEnemy(class Enemy* enemy);
    void RemoveEnemy(class Enemy* enemy);
    std::vector<class Enemy*>& GetEnemies() { return mEnemies; }

    void AddProjectile(class Projectile* projectile);
    void RemoveProjectile(class Projectile* projectile);
    std::vector<class Projectile*>& GetProjectiles() { return mProjectiles; }
    
    void SpawnExperienceOrb(const Vector2& position, float experienceValue);
    void AddExperienceOrb(class ExperienceOrb* orb);
    void RemoveExperienceOrb(class ExperienceOrb* orb);
    std::vector<class ExperienceOrb*>& GetExperienceOrbs() { return mExperienceOrbs; }
    
    // Deferred experience system - prevents crashes from state changes during updates
    struct DeferredExperience
    {
        float amount;
        Vector2 position;
    };
    void AddDeferredExperience(const DeferredExperience& deferred);

    void StartNewGame();
    void ResumeGame();
    void PauseGame();
    void QuitToMenu();
    void GameOver();
    void ShowUpgradeMenu();

    MenuState GetState() const { return mGameState; }
    void SetState(MenuState state) { mGameState = state; }

    int GetCurrentWave() const { return mCurrentWave; }
    float GetWaveTimer() const { return mWaveTimer; }

    // AJUSTADO: suporta projétil de inimigo e dano customizado
    void SpawnProjectile(const Vector2& position,
                         const Vector2& direction,
                         float speed,
                         bool fromPlayer = true,
                         float damage = 10.0f,
                         int pierce = 0,
                         bool homing = false,
                         bool explosive = false);

    void SpawnFallingParticles(const Vector2& position, const Vector3& color);
    void SpawnExplosionRing(const Vector2& position, float radius);
    void CreateDeathParticles(const Vector2& position, const Vector3& color, int count = 8);

    void AddBoss(class Boss* boss);
    void RemoveBoss(class Boss* boss);
    std::vector<class Boss*>& GetBosses() { return mBosses; }
    void SpawnBoss(int waveNumber);
    
    // Creative features
    void SpawnFloatingText(const Vector2& position, const std::string& text, const Vector3& color);
    void OnEnemyKilled(const Vector2& position);
    void AddKill();
    int GetKills() const { return mKills; }
    int GetCombo() const { return mCombo; }
    float GetComboMultiplier() const { return mComboMultiplier; }
    
private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void UpdateWaveSystem(float deltaTime);
    void SpawnEnemies(int count);
    void DrawUI();
    void CleanupGame();

    // Atores e Drawables
    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;
    std::vector<class DrawComponent*> mDrawables;

    SDL_Window* mWindow;
    class Renderer* mRenderer;
    AudioSystem* mAudioSystem;

    Uint32 mTicksCount;
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;
    Uint32 mLastPausePress;

    MenuState mGameState;
    class MainMenu* mMainMenu;
    class PauseMenu* mPauseMenu;
    class UpgradeMenu* mUpgradeMenu;

    class Player* mPlayer;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Projectile*> mProjectiles;
    std::vector<class ExperienceOrb*> mExperienceOrbs;
    
    // Deferred experience system - prevents crashes from state changes during updates
    std::vector<DeferredExperience> mDeferredExperience;

    // -------------------------------
    // NOVAS VARIÁVEIS DE WAVE/SPAWN
    // -------------------------------
    int mCurrentWave;
    float mWaveTimer;
    float mNextWaveTimer;
    int mEnemiesSpawned;
    int mEnemiesToSpawn;

    float mElapsedSeconds = 0.0f;
    std::vector<SpawnRule> mSpawnRules;
    std::vector<float> mRuleTimers;
    std::vector<TimedHorde> mTimedHordes;
    bool mBoss5Spawned = false;
    bool mBoss10Spawned = false;
    // -------------------------------

    Vector2 mCameraPosition;
    float mScreenShakeAmount;
    float mScreenShakeDuration;

    std::vector<class Boss*> mBosses;
    int mLastBossWaveSpawned;
    
    // Creative features - combo and kill tracking
    int mKills;
    int mCombo;
    float mComboTimer;
    float mComboMultiplier;
    static const float COMBO_TIMEOUT;
    static const float MAX_COMBO_MULTIPLIER;
};
