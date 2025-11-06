// ----------------------------------------------------------------
// Vampire Survivors-like Game
// ----------------------------------------------------------------

#include <algorithm>
#include <vector>
#include <string>
#include "Game.h"
#include "Actors/Player.h"
#include "Actors/Enemy.h"
#include "Actors/Projectile.h"
#include "Menus/MainMenu.h"
#include "Menus/PauseMenu.h"
#include "Menus/UpgradeMenu.h"
#include "Components/DrawComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CircleColliderComponent.h"
#include "Renderer/VertexArray.h"
#include "Renderer/TextRenderer.h"
#include "Random.h"

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mAudioSystem(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(false)
        ,mUpdatingActors(false)
        ,mGameState(MenuState::MainMenu)
        ,mMainMenu(nullptr)
        ,mPauseMenu(nullptr)
        ,mUpgradeMenu(nullptr)
        ,mPlayer(nullptr)
        ,mCurrentWave(1)
        ,mWaveTimer(0.0f)
        ,mNextWaveTimer(5.0f)
        ,mEnemiesSpawned(0)
        ,mEnemiesToSpawn(10)
        ,mCameraPosition(Vector2::Zero)
        ,mScreenShakeAmount(0.0f)
        ,mScreenShakeDuration(0.0f)
{

}

bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mAudioSystem = new AudioSystem();
    if (!mAudioSystem->Initialize())
    {
        SDL_Log("Failed to initialize audio system");
        return false;
    }

    mWindow = SDL_CreateWindow("Vampire Survivors", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize menus
    mMainMenu = new MainMenu(this);
    mPauseMenu = new PauseMenu(this);
    mUpgradeMenu = new UpgradeMenu(this);

    mAudioSystem->LoadMusic("menu", "Assets/Music/twd_theme.mp3");
    mAudioSystem->LoadMusic("gameplay", "Assets/Music/gameplay_music.mp3");

    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::InitializeActors()
{
    mPlayer = new Player(this);
    mPlayer->SetPosition(Vector2(static_cast<float>(WORLD_WIDTH) / 2.0f, static_cast<float>(WORLD_HEIGHT) / 2.0f));
    mCameraPosition = Vector2(static_cast<float>(WORLD_WIDTH) / 2.0f, static_cast<float>(WORLD_HEIGHT) / 2.0f);
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    // Handle menu input
    if (mGameState == MenuState::MainMenu)
    {
        mMainMenu->ProcessInput(state);
    }
    else if (mGameState == MenuState::Paused)
    {
        mPauseMenu->ProcessInput(state);
    }
    else if (mGameState == MenuState::UpgradeMenu)
    {
        mUpgradeMenu->ProcessInput(state);
    }
    else if (mGameState == MenuState::Playing)
    {
        // Pause game
        if (state[SDL_SCANCODE_ESCAPE])
        {
            PauseGame();
        }
        
        // Process actor input
    for (auto actor : mActors)
    {
        actor->ProcessInput(state);
        }
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if (mGameState == MenuState::Playing)
    {
    UpdateActors(deltaTime);
        UpdateWaveSystem(deltaTime);
        UpdateCamera(deltaTime);
    }
}

void Game::UpdateActors(float deltaTime)
{

    mUpdatingActors = true;
    

    for (auto actor : mActors)
    {
        actor->Update(deltaTime);
        
        // Clean up particles that are too far from camera
        Vector2 actorPos = actor->GetPosition();
        Vector2 cameraPos = mCameraPosition;
        float distance = Vector2::Distance(actorPos, cameraPos);
        
        // Destroy particles that are far away (assuming they're temporary particles)
        if (distance > 1000.0f && actor->GetState() == ActorState::Active)
        {
            // Check if it's a particle (has DrawComponent but no other identifying features)
            // For now, just clean up actors far from camera after a delay
            // This is a simple cleanup mechanism
        }
    }

    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto actor : mActors)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            deadActors.emplace_back(actor);
        }
    }

    for (auto actor : deadActors)
    {
        delete actor;
    }
}

void Game::AddEnemy(Enemy* enemy)
{
    mEnemies.emplace_back(enemy);
}

void Game::RemoveEnemy(Enemy* enemy)
{
    auto it = std::find(mEnemies.begin(), mEnemies.end(), enemy);
    if (it != mEnemies.end())
    {
        mEnemies.erase(it);
    }
}

void Game::AddProjectile(Projectile* projectile)
{
    mProjectiles.emplace_back(projectile);
}

void Game::RemoveProjectile(Projectile* projectile)
{
    auto it = std::find(mProjectiles.begin(), mProjectiles.end(), projectile);
    if (it != mProjectiles.end())
    {
        mProjectiles.erase(it);
    }
}

void Game::SpawnProjectile(const Vector2& position, const Vector2& direction, float speed)
{
    Projectile* projectile = new Projectile(this, position, direction, speed);
}

void Game::StartNewGame()
{
    if (mAudioSystem)
    {
        mAudioSystem->StopMusic();
        mAudioSystem->PlayMusic("gameplay", -1);
    }
    CleanupGame();
    InitializeActors();
    mGameState = MenuState::Playing;
    mCurrentWave = 1;
    mWaveTimer = 0.0f;
    mNextWaveTimer = 5.0f;
    mEnemiesSpawned = 0;
    mEnemiesToSpawn = 10;
}

void Game::ResumeGame()
{
    if (mGameState == MenuState::Paused || mGameState == MenuState::UpgradeMenu)
    {
        mGameState = MenuState::Playing;
        if (mAudioSystem)
        {
            mAudioSystem->ResumeMusic();
        }
    }
}

void Game::PauseGame()
{
    if (mGameState == MenuState::Playing)
    {
        mGameState = MenuState::Paused;
        if (mAudioSystem)
        {
            mAudioSystem->PauseMusic();
        }
    }
}

void Game::QuitToMenu()
{
    CleanupGame();
    mGameState = MenuState::MainMenu;
    if (mAudioSystem)
    {
        mAudioSystem->StopMusic();
        mAudioSystem->PlayMusic("menu", -1);
    }
}

void Game::GameOver()
{
    mGameState = MenuState::GameOver;
}

void Game::ShowUpgradeMenu()
{
    mGameState = MenuState::UpgradeMenu;
    mUpgradeMenu->GenerateUpgrades();
}

void Game::UpdateCamera(float deltaTime)
{
    if (!mPlayer)
        return;
    
    Vector2 playerPos = mPlayer->GetPosition();
    
    // Smoothly follow the player (center camera on player)
    Vector2 targetCameraPos = playerPos;
    
    // Smooth camera movement
    Vector2 diff = targetCameraPos - mCameraPosition;
    mCameraPosition += diff * (deltaTime * 5.0f); // 5.0f is camera follow speed
    
    // Apply screen shake
    if (mScreenShakeDuration > 0.0f)
    {
        mScreenShakeDuration -= deltaTime;
        if (mScreenShakeDuration <= 0.0f)
        {
            mScreenShakeAmount = 0.0f;
            mScreenShakeDuration = 0.0f;
        }
        
        // Add random shake offset
        float shakeX = Random::GetFloatRange(-mScreenShakeAmount, mScreenShakeAmount);
        float shakeY = Random::GetFloatRange(-mScreenShakeAmount, mScreenShakeAmount);
        mCameraPosition += Vector2(shakeX, shakeY);
    }
}

void Game::AddScreenShake(float intensity, float duration)
{
    mScreenShakeAmount = intensity;
    mScreenShakeDuration = duration;
}

void Game::SpawnDeathParticles(const Vector2& position, const Vector3& color)
{
    CreateDeathParticles(position, color, 16); // More particles for cooler effect
}

void Game::CreateDeathParticles(const Vector2& position, const Vector3& color, int count)
{
    // Create small particles for death effect
    std::vector<Vector2> particleVertices;
    // Create a small square particle
    float size = Random::GetFloatRange(1.5f, 3.0f);
    particleVertices.emplace_back(Vector2(-size, -size));
    particleVertices.emplace_back(Vector2(size, -size));
    particleVertices.emplace_back(Vector2(size, size));
    particleVertices.emplace_back(Vector2(-size, size));
    
    for (int i = 0; i < count; ++i)
    {
        float angle = Random::GetFloatRange(0.0f, Math::TwoPi); // Random angle for more chaotic effect
        float speed = Random::GetFloatRange(80.0f, 250.0f);
        Vector2 dir(Math::Cos(angle), Math::Sin(angle));
        Vector2 particlePos = position + dir * Random::GetFloatRange(0.0f, 5.0f);
        
        // Create temporary particle actor
        Actor* particle = new Actor(this);
        particle->SetPosition(particlePos);
        
        DrawComponent* drawComp = new DrawComponent(particle, particleVertices);
        // Vary particle colors slightly for more visual interest
        Vector3 particleColor = color;
        particleColor.x = Math::Clamp(particleColor.x + Random::GetFloatRange(-0.2f, 0.2f), 0.0f, 1.0f);
        particleColor.y = Math::Clamp(particleColor.y + Random::GetFloatRange(-0.2f, 0.2f), 0.0f, 1.0f);
        particleColor.z = Math::Clamp(particleColor.z + Random::GetFloatRange(-0.2f, 0.2f), 0.0f, 1.0f);
        drawComp->SetColor(particleColor);
        drawComp->SetFilled(true);
        drawComp->SetUseCamera(true);
        
        RigidBodyComponent* rbComp = new RigidBodyComponent(particle, 0.1f);
        rbComp->SetVelocity(dir * speed);
        
        // Particles will be cleaned up naturally when they go out of bounds or after a delay
        // We'll handle cleanup in UpdateActors - for now just mark as active
        particle->SetState(ActorState::Active);
    }
}

void Game::UpdateWaveSystem(float deltaTime)
{
    mWaveTimer += deltaTime;
    
    // Vampire Survivors-style: continuous spawning based on time
    // Spawn rate increases over time - much faster now!
    float spawnRate = 0.3f - (mWaveTimer * 0.0005f); // Gets faster over time
    if (spawnRate < 0.05f) spawnRate = 0.05f; // Minimum spawn rate
    
    static float spawnTimer = 0.0f;
    spawnTimer += deltaTime;
    
    // Spawn enemies continuously - WAY MORE enemies!
    int maxEnemies = 300 + (mCurrentWave * 20); // Much higher limit
    if (spawnTimer >= spawnRate && mEnemies.size() < maxEnemies)
    {
        // Spawn way more enemies per wave
        int spawnCount = 3 + (mCurrentWave * 2); // Many more enemies per spawn
        if (spawnCount > 50) spawnCount = 50; // Cap at 50 per spawn
        SpawnEnemies(spawnCount);
        spawnTimer = 0.0f;
    }
    
    // Update wave number based on time (every 30 seconds)
    int newWave = 1 + static_cast<int>(mWaveTimer / 30.0f);
    if (newWave > mCurrentWave)
    {
        mCurrentWave = newWave;
    }
}

void Game::SpawnEnemies(int count)
{
    if (!mPlayer)
        return;

    Vector2 playerPos = mPlayer->GetPosition();
    
    for (int i = 0; i < count; ++i)
    {
        Vector2 spawnPos;
        
        // Spawn enemies around player at a distance (world coordinates)
        float spawnDistance = 350.0f + Random::GetFloatRange(-50.0f, 50.0f); // Variable distance
        float angle = Random::GetFloatRange(0.0f, Math::TwoPi);
        
        spawnPos.x = playerPos.x + Math::Cos(angle) * spawnDistance;
        spawnPos.y = playerPos.y + Math::Sin(angle) * spawnDistance;
        
        // Clamp to world bounds
        float radius = 20.0f;
        if (spawnPos.x < radius) spawnPos.x = radius;
        if (spawnPos.x > WORLD_WIDTH - radius) spawnPos.x = WORLD_WIDTH - radius;
        if (spawnPos.y < radius) spawnPos.y = radius;
        if (spawnPos.y > WORLD_HEIGHT - radius) spawnPos.y = WORLD_HEIGHT - radius;
        
        // Enemies get stronger over time
        float enemyRadius = 12.0f + (mCurrentWave * 0.5f);
        float speed = 70.0f + (mCurrentWave * 3.0f);
        float health = 25.0f + (mCurrentWave * 8.0f);
        
        Enemy* enemy = new Enemy(this, enemyRadius, speed, health);
        enemy->SetPosition(spawnPos);
    }
}

void Game::DrawUI()
{
    if (!mPlayer)
        return;

    // Draw health bar
    float healthPercent = mPlayer->GetHealth() / mPlayer->GetMaxHealth();
    // Helper lambda to create VertexArray from Vector2 vertices
    auto createVA = [](const std::vector<Vector2>& vertices) -> VertexArray* {
        std::vector<float> floatArray;
        std::vector<unsigned int> indices;
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            floatArray.push_back(vertices[i].x);
            floatArray.push_back(vertices[i].y);
            floatArray.push_back(0.0f);
            indices.push_back(static_cast<unsigned int>(i));
        }
        return new VertexArray(floatArray.data(), static_cast<unsigned int>(vertices.size()), indices.data(), static_cast<unsigned int>(indices.size()));
    };
    
    std::vector<Vector2> healthBg;
    healthBg.emplace_back(Vector2(20.0f, 20.0f));
    healthBg.emplace_back(Vector2(220.0f, 20.0f));
    healthBg.emplace_back(Vector2(220.0f, 40.0f));
    healthBg.emplace_back(Vector2(20.0f, 40.0f));
    
    Matrix4 healthBgMatrix = Matrix4::Identity;
    VertexArray* healthBgVA = createVA(healthBg);
    Vector3 healthBgColor(0.2f, 0.2f, 0.2f);
    mRenderer->Draw(healthBgMatrix, healthBgVA, healthBgColor);
    delete healthBgVA;
    
    std::vector<Vector2> healthBar;
    healthBar.emplace_back(Vector2(20.0f, 20.0f));
    healthBar.emplace_back(Vector2(20.0f + 200.0f * healthPercent, 20.0f));
    healthBar.emplace_back(Vector2(20.0f + 200.0f * healthPercent, 40.0f));
    healthBar.emplace_back(Vector2(20.0f, 40.0f));
    
    Matrix4 healthMatrix = Matrix4::Identity;
    VertexArray* healthVA = createVA(healthBar);
    Vector3 healthColor(1.0f - healthPercent, healthPercent, 0.0f);
    mRenderer->Draw(healthMatrix, healthVA, healthColor);
    delete healthVA;
    
    // Draw experience bar
    float expPercent = mPlayer->GetExperience() / mPlayer->GetExperienceToNextLevel();
    std::vector<Vector2> expBg;
    expBg.emplace_back(Vector2(20.0f, 50.0f));
    expBg.emplace_back(Vector2(220.0f, 50.0f));
    expBg.emplace_back(Vector2(220.0f, 60.0f));
    expBg.emplace_back(Vector2(20.0f, 60.0f));
    
    Matrix4 expBgMatrix = Matrix4::Identity;
    VertexArray* expBgVA = createVA(expBg);
    Vector3 expBgColor(0.2f, 0.2f, 0.2f);
    mRenderer->Draw(expBgMatrix, expBgVA, expBgColor);
    delete expBgVA;
    
    std::vector<Vector2> expBar;
    expBar.emplace_back(Vector2(20.0f, 50.0f));
    expBar.emplace_back(Vector2(20.0f + 200.0f * expPercent, 50.0f));
    expBar.emplace_back(Vector2(20.0f + 200.0f * expPercent, 60.0f));
    expBar.emplace_back(Vector2(20.0f, 60.0f));
    
    Matrix4 expMatrix = Matrix4::Identity;
    VertexArray* expVA = createVA(expBar);
    Vector3 expColor(0.2f, 0.8f, 1.0f);
    mRenderer->Draw(expMatrix, expVA, expColor);
    delete expVA;
    
    // Draw wave counter
    std::vector<Vector2> waveBg;
    waveBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) - 150.0f, 20.0f));
    waveBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) - 20.0f, 20.0f));
    waveBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) - 20.0f, 60.0f));
    waveBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) - 150.0f, 60.0f));
    
    Matrix4 waveMatrix = Matrix4::Identity;
    VertexArray* waveVA = createVA(waveBg);
    Vector3 waveColor(0.3f, 0.3f, 0.4f);
    mRenderer->Draw(waveMatrix, waveVA, waveColor);
    delete waveVA;
    
    // Draw text labels
    TextRenderer::DrawText(mRenderer, "HP", Vector2(25.0f, 5.0f), 0.7f, Vector3(1.0f, 0.5f, 0.5f));
    TextRenderer::DrawText(mRenderer, "EXP", Vector2(25.0f, 45.0f), 0.7f, Vector3(0.5f, 0.8f, 1.0f));
    
    // Draw wave text
    std::string waveText = "WAVE " + std::to_string(mCurrentWave);
    TextRenderer::DrawText(mRenderer, waveText, Vector2(static_cast<float>(WINDOW_WIDTH) - 140.0f, 35.0f), 0.8f, Vector3(1.0f, 1.0f, 1.0f));
    
    // Draw level text
    std::string levelText = "LVL " + std::to_string(mPlayer->GetLevel());
    TextRenderer::DrawText(mRenderer, levelText, Vector2(250.0f, 25.0f), 0.8f, Vector3(1.0f, 1.0f, 0.0f));
}

void Game::CleanupGame()
{
    // Remove all actors
    while (!mActors.empty())
    {
        delete mActors.back();
    }
    
    mEnemies.clear();
    mProjectiles.clear();
    mPlayer = nullptr;
}

void Game::AddActor(Actor* actor)
{

    if (mUpdatingActors)
    {
        mPendingActors.emplace_back(actor);
    }
    else
    {

        mActors.emplace_back(actor);
    }
}

void Game::RemoveActor(Actor* actor)
{

    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {

        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }


    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {

        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);

    std::sort(mDrawables.begin(), mDrawables.end(), [](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    auto iter = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    if (iter != mDrawables.end())
    {
        mDrawables.erase(iter);
    }
}

void Game::GenerateOutput()
{
    mRenderer->Clear();

    if (mGameState == MenuState::MainMenu)
    {
        mMainMenu->Draw(mRenderer);
    }
    else if (mGameState == MenuState::Paused)
    {
        // Draw game in background
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }
        DrawUI();
        mPauseMenu->Draw(mRenderer);
    }
    else if (mGameState == MenuState::UpgradeMenu)
    {
        // Draw game in background
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }
        DrawUI();
        mUpgradeMenu->Draw(mRenderer);
    }
    else if (mGameState == MenuState::Playing)
    {
        // Draw all game objects
    for (auto drawable : mDrawables)
    {
        drawable->Draw(mRenderer);
    }
    
        DrawUI();

    if (mIsDebugging)
    {
        for (auto actor : mActors)
        {
            for (auto component : actor->GetComponents())
            {
                component->DebugDraw(mRenderer);
            }
        }
    }
    }
    else if (mGameState == MenuState::GameOver)
    {
        // Draw game in background
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }
        DrawUI();
        
        // Draw game over screen
        std::vector<Vector2> gameOverBg;
        gameOverBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) / 2.0f - 200.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f - 100.0f));
        gameOverBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) / 2.0f + 200.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f - 100.0f));
        gameOverBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) / 2.0f + 200.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f + 100.0f));
        gameOverBg.emplace_back(Vector2(static_cast<float>(WINDOW_WIDTH) / 2.0f - 200.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f + 100.0f));
        
        std::vector<float> goFloatArray;
        std::vector<unsigned int> goIndices;
        for (size_t i = 0; i < gameOverBg.size(); ++i)
        {
            goFloatArray.push_back(gameOverBg[i].x);
            goFloatArray.push_back(gameOverBg[i].y);
            goFloatArray.push_back(0.0f);
            goIndices.push_back(static_cast<unsigned int>(i));
        }
        
        Matrix4 gameOverMatrix = Matrix4::Identity;
        VertexArray gameOverVA(goFloatArray.data(), static_cast<unsigned int>(gameOverBg.size()), goIndices.data(), static_cast<unsigned int>(goIndices.size()));
        Vector3 gameOverColor(0.8f, 0.2f, 0.2f);
        mRenderer->Draw(gameOverMatrix, &gameOverVA, gameOverColor);
    }

    mRenderer->Present();
}

void Game::Shutdown()
{
    CleanupGame();
    
    delete mMainMenu;
    delete mPauseMenu;
    delete mUpgradeMenu;

    if (mAudioSystem)
    {
        mAudioSystem->Shutdown();
        delete mAudioSystem;
        mAudioSystem = nullptr;
    }

    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}