#pragma once
#include <SDL.h>
#include <vector>
#include "Renderer/Renderer.h"
#include "Audio/AudioSystem.h"

// ============================================
//  NOVOS ENUMS E ESTRUTURAS DE SPAWN
// ============================================
enum class EnemyKind { Comum, Corredor, GordoExplosivo, Atirador };

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
    void SpawnEnemyOfKind(EnemyKind kind);
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
    void SpawnDeathParticles(const Vector2& position, const Vector3& color);

    void AddEnemy(class Enemy* enemy);
    void RemoveEnemy(class Enemy* enemy);
    std::vector<class Enemy*>& GetEnemies() { return mEnemies; }

    void AddProjectile(class Projectile* projectile);
    void RemoveProjectile(class Projectile* projectile);
    std::vector<class Projectile*>& GetProjectiles() { return mProjectiles; }

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

    void SpawnProjectile(const Vector2& position, const Vector2& direction, float speed);

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

    MenuState mGameState;
    class MainMenu* mMainMenu;
    class PauseMenu* mPauseMenu;
    class UpgradeMenu* mUpgradeMenu;

    class Player* mPlayer;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Projectile*> mProjectiles;

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

    void CreateDeathParticles(const Vector2& position, const Vector3& color, int count = 8);
};
