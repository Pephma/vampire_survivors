// ----------------------------------------------------------------
// Vampire Survivors-like Game
// ----------------------------------------------------------------

#pragma once
#include <SDL.h>
#include <vector>
#include "Renderer/Renderer.h"
#include "Audio/AudioSystem.h"

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

    // Actor functions
    void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);

    class Renderer* GetRenderer() { return mRenderer; }
    AudioSystem* GetAudioSystem() { return mAudioSystem; }

    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;
    static const int WORLD_WIDTH = 4000;  // Large world
    static const int WORLD_HEIGHT = 4000; // Large world

    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    // Player access
    class Player* GetPlayer() const { return mPlayer; }
    
    // Camera
    Vector2 GetCameraPosition() const { return mCameraPosition; }
    void UpdateCamera(float deltaTime);
    void AddScreenShake(float intensity, float duration = 0.2f);
    void SpawnDeathParticles(const Vector2& position, const Vector3& color);
    
    // Enemy management
    void AddEnemy(class Enemy* enemy);
    void RemoveEnemy(class Enemy* enemy);
    std::vector<class Enemy*>& GetEnemies() { return mEnemies; }

    // Projectile management
    void AddProjectile(class Projectile* projectile);
    void RemoveProjectile(class Projectile* projectile);
    std::vector<class Projectile*>& GetProjectiles() { return mProjectiles; }

    // Game state management
    void StartNewGame();
    void ResumeGame();
    void PauseGame();
    void QuitToMenu();
    void GameOver();
    void ShowUpgradeMenu();
    
    MenuState GetState() const { return mGameState; }
    void SetState(MenuState state) { mGameState = state; }
    
    // Wave system
    int GetCurrentWave() const { return mCurrentWave; }
    float GetWaveTimer() const { return mWaveTimer; }

    // Projectile spawning
    void SpawnProjectile(const Vector2& position, const Vector2& direction, float speed);

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void UpdateWaveSystem(float deltaTime);
    void SpawnEnemies(int count);
    void DrawUI();
    void CleanupGame();

    // All the actors in the game
    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;

    // All the draw components
    std::vector<class DrawComponent*> mDrawables;

    // SDL stuff
    SDL_Window* mWindow;
    class Renderer* mRenderer;
    AudioSystem* mAudioSystem;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;

    // Game state
    MenuState mGameState;
    
    // Menus
    class MainMenu* mMainMenu;
    class PauseMenu* mPauseMenu;
    class UpgradeMenu* mUpgradeMenu;

    // Game-specific
    class Player* mPlayer;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Projectile*> mProjectiles;
    
    // Wave system
    int mCurrentWave;
    float mWaveTimer;
    float mNextWaveTimer;
    int mEnemiesSpawned;
    int mEnemiesToSpawn;
    
    // Camera
    Vector2 mCameraPosition;
    float mScreenShakeAmount;
    float mScreenShakeDuration;
    
    // Particle effects
    void CreateDeathParticles(const Vector2& position, const Vector3& color, int count = 8);
};
