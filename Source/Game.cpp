// ----------------------------------------------------------------
// Vampire Survivors-like Game
// ----------------------------------------------------------------

#include <algorithm>
#include <vector>
#include <string>
#include <limits> // std::numeric_limits<float>::infinity()

#include "Game.h"
#include "Actors/Player.h"
#include "Actors/Enemy.h"
#include "Actors/Projectile.h"
#include "Actors/Boss.h" // <-- INCLUÍDO O CHEFE
#include "Actors/ExperienceOrb.h"
#include "Actors/FloatingText.h"
#include "Menus/MainMenu.h"
#include "Menus/PauseMenu.h"
#include "Menus/UpgradeMenu.h"
#include "Components/DrawComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CircleColliderComponent.h"
#include "Renderer/VertexArray.h"
#include "Renderer/TextRenderer.h"
#include "Renderer/Renderer.h"
#include "Random.h"
#include "Math.h"
#include <SDL.h>

// Static const member definitions
const float Game::COMBO_TIMEOUT = 3.0f;
const float Game::MAX_COMBO_MULTIPLIER = 5.0f;

Game::Game()
        : mWindow(nullptr)
        , mRenderer(nullptr)
        , mAudioSystem(nullptr)
        , mTicksCount(0)
        , mIsRunning(true)
        , mIsDebugging(false)
        , mUpdatingActors(false)
        , mGameState(MenuState::MainMenu)
        , mMainMenu(nullptr)
        , mPauseMenu(nullptr)
        , mUpgradeMenu(nullptr)
        , mPlayer(nullptr)
        , mCurrentWave(1)
        , mWaveTimer(0.0f)
        , mNextWaveTimer(5.0f)
        , mEnemiesSpawned(0)
        , mEnemiesToSpawn(10)
        , mElapsedSeconds(0.0f)
        , mBoss5Spawned(false) // (Não mais usado pela nova lógica)
        , mBoss10Spawned(false) // (Não mais usado pela nova lógica)
        , mCameraPosition(Vector2::Zero)
        , mScreenShakeAmount(0.0f)
        , mScreenShakeDuration(0.0f)
        , mLastBossWaveSpawned(0) // <-- ADICIONADO PARA O CHEFE
        , mLastPausePress(0)
        , mKills(0)
        , mCombo(0)
        , mComboTimer(0.0f)
        , mComboMultiplier(1.0f)
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

    mWindow = SDL_CreateWindow("Foge, Sô!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Initialize text renderer
    TextRenderer::Initialize();

    // Initialize menus
    mMainMenu  = new MainMenu(this);
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
    else if (mGameState == MenuState::GameOver)
    {
        if (state[SDL_SCANCODE_SPACE] || state[SDL_SCANCODE_ESCAPE] || state[SDL_SCANCODE_RETURN])
        {
            QuitToMenu();
        }
    }
    else if (mGameState == MenuState::Playing)
    {
        if (state[SDL_SCANCODE_ESCAPE])
        {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - mLastPausePress > 300)
            {
                PauseGame();
                mLastPausePress = currentTime;
            }
        }
        else
        {
            if (SDL_GetTicks() - mLastPausePress > 500)
            {
                mLastPausePress = 0;
            }
        }

        if (state[SDL_SCANCODE_U])
        {
            if (mPlayer && mPlayer->GetPendingUpgrades() > 0)
            {
                ShowUpgradeMenu();
            }
        }

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
        if (mPlayer && mPlayer->GetHealth() > 0.0f)
        {
            UpdateCamera(deltaTime);
        }
    }
    else if (mGameState == MenuState::UpgradeMenu)
    {
        // Pause game updates but keep camera following player
        if (mPlayer)
        {
            UpdateCamera(deltaTime);
        }
    }
    else if (mGameState == MenuState::GameOver)
    {
        // Don't update game logic when game is over
        // Only update camera to show final position
        if (mPlayer)
        {
            UpdateCamera(deltaTime);
        }
    }
}

void Game::UpdateActors(float deltaTime)
{
    // Don't update actors if we're not in Playing state
    if (mGameState != MenuState::Playing)
    {
        return;
    }
    
    // Cap deltaTime to prevent huge jumps that could cause issues
    if (deltaTime > 0.033f)  // Cap at ~30 FPS minimum
    {
        deltaTime = 0.033f;
    }
    
    // Update combo system
    if (mComboTimer > 0.0f)
    {
        mComboTimer -= deltaTime;
        if (mComboTimer <= 0.0f)
        {
            // Combo expired
            if (mCombo > 0)
            {
                SpawnFloatingText(mPlayer ? mPlayer->GetPosition() : Vector2(WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f), 
                    "COMBO ENDED!", Vector3(0.8f, 0.2f, 0.2f));
            }
            mCombo = 0;
            mComboMultiplier = 1.0f;
        }
    }
    
    mUpdatingActors = true;

    // Create a copy of actors to iterate over safely
    // This prevents iterator invalidation if actors are added/removed during update
    std::vector<Actor*> actorsCopy = mActors;

    for (auto actor : actorsCopy)
    {
        // Check state again in case it changed during update
        if (mGameState != MenuState::Playing)
        {
            break;
        }
        
        // Safety checks
        if (!actor) continue;
        
        // Check if actor is still in the main list (might have been removed)
        auto it = std::find(mActors.begin(), mActors.end(), actor);
        if (it == mActors.end())
        {
            continue;  // Actor was removed, skip it
        }
        
        // Don't update if already destroyed
        if (actor->GetState() == ActorState::Destroy)
        {
            continue;
        }
        
        actor->Update(deltaTime);
        
        // Check state again after update - it might have changed
        if (mGameState != MenuState::Playing)
        {
            break;
        }
    }

    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();
    
    // Process deferred experience AFTER all updates are done
    // This prevents state changes during actor updates
    if (mPlayer && !mDeferredExperience.empty())
    {
        for (const auto& deferred : mDeferredExperience)
        {
            mPlayer->AddExperience(deferred.amount);
            // No particles - just give experience
        }
        mDeferredExperience.clear();
    }

    // CRITICAL: Process dead actors immediately to prevent corruption
    // Collect indices of actors to delete first, then delete them
    // This avoids iterator invalidation and accessing corrupted memory
    std::vector<size_t> indicesToDelete;
    std::vector<Actor*> actorsToDelete;
    
    // First pass: identify actors to delete
    for (size_t i = 0; i < mActors.size(); ++i)
    {
        Actor* actor = mActors[i];
        if (!actor) 
        {
            indicesToDelete.push_back(i);
            continue;
        }
        
        // Check state - if destroyed, mark for deletion
        // Simple approach: just check state directly
        // If actor is corrupted, the crash will happen here, but at least we've isolated it
        if (actor->GetState() == ActorState::Destroy)
        {
            indicesToDelete.push_back(i);
            actorsToDelete.push_back(actor);
            
            // Remove from experience orbs list if it's an ExperienceOrb (before deletion)
            ExperienceOrb* orb = dynamic_cast<ExperienceOrb*>(actor);
            if (orb)
            {
                RemoveExperienceOrb(orb);
            }
        }
    }
    
    // Second pass: remove from mActors (iterate backwards to maintain indices)
    for (int i = static_cast<int>(indicesToDelete.size()) - 1; i >= 0; --i)
    {
        size_t idx = indicesToDelete[i];
        if (idx < mActors.size())
        {
            mActors.erase(mActors.begin() + idx);
        }
    }
    
    // Third pass: delete the actors
    // Their destructors will call RemoveActor, but since we've already removed them
    // and mUpdatingActors is true, RemoveActor will safely return
    for (auto actor : actorsToDelete)
    {
        if (actor)
        {
            delete actor;
        }
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

// --- FUNÇÕES DO CHEFE ADICIONADAS ---
void Game::AddBoss(Boss* boss)
{
    mBosses.emplace_back(boss);
}

void Game::RemoveBoss(Boss* boss)
{
    auto it = std::find(mBosses.begin(), mBosses.end(), boss);
    if (it != mBosses.end())
    {
        mBosses.erase(it);
    }
}


void Game::SpawnBoss(int waveNumber)
{
    // --- LÓGICA DE ESCOLHA DO CHEFE ---
    BossKind kind;

    if (waveNumber == 5)
    {
        kind = BossKind::Tank;
    }
    else if (waveNumber == 10)
    {
        kind = BossKind::Sprayer;
    }
    else
    {
        // Padrão para waves 15, 20, 25... (repete o Tank, mas mais forte)
        kind = BossKind::Tank;
    }
    // ---------------------------------

    // Spawna o chefe (ex: no meio superior da tela)
    Vector2 playerPos = mPlayer ? mPlayer->GetPosition() : Vector2(WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f);
    Vector2 spawnPos(playerPos.x, playerPos.y - 400.0f); // Spawna acima do jogador

    // --- MODIFICADO ---
    // Passa o 'kind' e 'waveNumber' para o construtor
    Boss* boss = new Boss(this, kind, waveNumber);
    // ------------------

    boss->SetPosition(spawnPos);
}
// ------------------------------------

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

void Game::SpawnProjectile(const Vector2& position,
                           const Vector2& direction,
                           float speed,
                           bool fromPlayer,
                           float damage,
                           int pierce,
                           bool homing,
                           bool explosive)
{
    Projectile* projectile = new Projectile(this, position, direction, speed, fromPlayer, damage, pierce, homing, explosive);
}

void Game::SpawnExperienceOrb(const Vector2& position, float experienceValue)
{
    ExperienceOrb* orb = new ExperienceOrb(this, position, experienceValue);
    AddExperienceOrb(orb);
    // Note: Don't call AddActor here - the Actor constructor already calls AddActor(this)
    // Calling it twice causes duplicate entries and crashes
}

void Game::AddExperienceOrb(ExperienceOrb* orb)
{
    mExperienceOrbs.emplace_back(orb);
}

void Game::RemoveExperienceOrb(ExperienceOrb* orb)
{
    if (!orb) return;
    
    // Safely remove from list using manual iteration to avoid any issues
    // This is more defensive than erase-remove and handles edge cases better
    for (auto it = mExperienceOrbs.begin(); it != mExperienceOrbs.end(); )
    {
        if (*it == orb)
        {
            it = mExperienceOrbs.erase(it);
            return;  // Found and removed, exit immediately
        }
        else
        {
            ++it;
        }
    }
}

void Game::AddDeferredExperience(const DeferredExperience& deferred)
{
    mDeferredExperience.emplace_back(deferred);
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

    // NOVO: reset e inicialização do sistema de spawn
    mElapsedSeconds = 0.0f;
    InitSpawnRules();

    // Reseta a lógica do chefe
    mBosses.clear();
    mLastBossWaveSpawned = 0;
    mKills = 0;
    mCombo = 0;
    mComboTimer = 0.0f;
    mComboMultiplier = 1.0f;

    //para spawnar o boss tank descomentar e adicionar valor 1 ou para o boss spray valor 10 nas 2 linhas seguintes
    // SpawnBoss(1);
    // mLastBossWaveSpawned = 1;
    // ----------------------------------------
}

void Game::SpawnFloatingText(const Vector2& position, const std::string& text, const Vector3& color)
{
    FloatingText* floatingText = new FloatingText(this, position, text, color);
    AddActor(floatingText);
}

void Game::OnEnemyKilled(const Vector2& position)
{
    // TEMPORARILY DISABLED: Combo system disabled to prevent crashes
    // Will re-enable once we fix the memory corruption issue
    /*
    // Safety check - only process if game is in playing state
    if (mGameState != MenuState::Playing)
    {
        return;
    }
    
    AddKill();
    
    // Reset combo timer
    mComboTimer = COMBO_TIMEOUT;
    mCombo++;
    
    // Calculate combo multiplier (capped at MAX_COMBO_MULTIPLIER)
    mComboMultiplier = 1.0f + (mCombo * 0.05f); // +5% per kill
    if (mComboMultiplier > MAX_COMBO_MULTIPLIER)
    {
        mComboMultiplier = MAX_COMBO_MULTIPLIER;
    }
    
    // Visual feedback for combos - only spawn if we have a valid renderer
    if (mRenderer && mCombo > 0)
    {
        if (mCombo % 10 == 0)
        {
            SpawnFloatingText(position, "x" + std::to_string(mCombo) + " COMBO!", Vector3(1.0f, 0.8f, 0.0f));
            AddScreenShake(3.0f, 0.2f);
        }
        else if (mCombo % 5 == 0)
        {
            SpawnFloatingText(position, "x" + std::to_string(mCombo), Vector3(1.0f, 0.6f, 0.2f));
        }
    }
    */
    
    // Just track kills for now, no combo system
    AddKill();
}

void Game::AddKill()
{
    mKills++;
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
    // Change state first to prevent any updates during cleanup
    mGameState = MenuState::MainMenu;

    mMainMenu->ResetInput();

    // Always cleanup, but check if we're updating actors
    if (!mUpdatingActors)
    {
        CleanupGame();
    }
    else
    {
        // If we're updating actors, defer cleanup
        // But clear the state references immediately
        mPlayer = nullptr;
        mEnemies.clear();
        mProjectiles.clear();
        mBosses.clear();
    }

    if (mAudioSystem)
    {
        mAudioSystem->StopMusic();
        mAudioSystem->PlayMusic("menu", -1);
    }
}

void Game::GameOver()
{
    if (mGameState != MenuState::Playing)
    {
        return;
    }
    
    mGameState = MenuState::GameOver;
    
    // Stop all player updates and movement
    if (mPlayer)
    {
        mPlayer->SetState(ActorState::Paused);
    }
    
    // Stop all enemies from moving/chasing - they'll stop updating in their OnUpdate
    // The state check in Enemy::OnUpdate will prevent them from chasing
    
    // Stop wave system updates
    // (Already handled by UpdateGame checking mGameState)
    
    if (mAudioSystem)
    {
        mAudioSystem->StopMusic();
    }
}

void Game::ShowUpgradeMenu()
{
    // Prevent showing upgrade menu if already showing or not in playing state
    if (mGameState != MenuState::Playing)
    {
        return;
    }
    
    mGameState = MenuState::UpgradeMenu;
    mUpgradeMenu->GenerateUpgrades();
}

void Game::UpdateCamera(float deltaTime)
{
    if (!mPlayer)
        return;

    Vector2 playerPos = mPlayer->GetPosition();

    // Smooth camera follow with better feel
    Vector2 targetCameraPos = playerPos;

    // Smooth camera movement - faster response for better feel
    Vector2 diff = targetCameraPos - mCameraPosition;
    mCameraPosition += diff * (deltaTime * 10.0f); // Even snappier camera for better responsiveness

    // Screen shake with better decay curve
    if (mScreenShakeDuration > 0.0f)
    {
        float shakeX = Random::GetFloatRange(-mScreenShakeAmount, mScreenShakeAmount);
        float shakeY = Random::GetFloatRange(-mScreenShakeAmount, mScreenShakeAmount);
        mCameraPosition.x += shakeX;
        mCameraPosition.y += shakeY;
        
        mScreenShakeDuration -= deltaTime;
        // Better decay curve - exponential falloff
        mScreenShakeAmount *= (1.0f - deltaTime * 10.0f);  // Faster decay for snappier feel
        if (mScreenShakeDuration <= 0.0f || mScreenShakeAmount < 0.1f)
        {
            mScreenShakeAmount = 0.0f;
            mScreenShakeDuration = 0.0f;
        }
    }
}

void Game::AddScreenShake(float intensity, float duration)
{
    mScreenShakeAmount = intensity;
    mScreenShakeDuration = duration;
}

// RENOMEADO: Esta função agora é específica para a explosão
void Game::SpawnExplosionParticles(const Vector2& position, const Vector3& color)
{
    CreateDeathParticles(position, color, 30);  // Even more particles for spectacular feel
    AddScreenShake(3.0f, 0.15f);  // Add screen shake for better feedback
}

void Game::CreateDeathParticles(const Vector2& position, const Vector3& color, int count)
{
    // Create small particles for death effect with better visuals
    std::vector<Vector2> particleVertices;
    float size = Random::GetFloatRange(2.0f, 4.0f);  // Slightly larger particles
    particleVertices.emplace_back(Vector2(-size, -size));
    particleVertices.emplace_back(Vector2(size, -size));
    particleVertices.emplace_back(Vector2(size, size));
    particleVertices.emplace_back(Vector2(-size, size));

    for (int i = 0; i < count; ++i)
    {
        float angle = Random::GetFloatRange(0.0f, Math::TwoPi);
        float speed = Random::GetFloatRange(100.0f, 300.0f);  // Faster particles
        Vector2 dir(Math::Cos(angle), Math::Sin(angle));
        Vector2 particlePos = position + dir * Random::GetFloatRange(0.0f, 8.0f);  // Better spread

        Actor* particle = new Actor(this);
        particle->SetPosition(particlePos);

        DrawComponent* drawComp = new DrawComponent(particle, particleVertices);
        Vector3 particleColor = color;
        // More vibrant color variation
        particleColor.x = Math::Clamp(particleColor.x + Random::GetFloatRange(-0.3f, 0.3f), 0.0f, 1.0f);
        particleColor.y = Math::Clamp(particleColor.y + Random::GetFloatRange(-0.3f, 0.3f), 0.0f, 1.0f);
        particleColor.z = Math::Clamp(particleColor.z + Random::GetFloatRange(-0.3f, 0.3f), 0.0f, 1.0f);
        drawComp->SetColor(particleColor);
        drawComp->SetFilled(true);
        drawComp->SetUseCamera(true);

        RigidBodyComponent* rbComp = new RigidBodyComponent(particle, 0.1f);
        rbComp->SetVelocity(dir * speed);

        particle->SetState(ActorState::Active);
        particle->SetLifetime(0.8f); // Longer lifetime for spectacular visibility
    }
}


// NOVO: Efeito de partículas "caindo" para inimigos comuns
void Game::SpawnFallingParticles(const Vector2& position, const Vector3& color)
{
    // Cria partículas pequenas (quadrados)
    std::vector<Vector2> particleVertices;
    float size = Random::GetFloatRange(1.5f, 3.0f);
    particleVertices.emplace_back(Vector2(-size, -size));
    particleVertices.emplace_back(Vector2(size, -size));
    particleVertices.emplace_back(Vector2(size, size));
    particleVertices.emplace_back(Vector2(-size, size));

    int count = 18; // More particles for spectacular visual feedback

    for (int i = 0; i < count; ++i)
    {
        // Direção: X aleatório, Y predominantemente para baixo (simulando "cair")
        float dirX = Random::GetFloatRange(-0.6f, 0.6f);
        float dirY = Random::GetFloatRange(0.4f, 1.0f); // Positivo = para baixo
        float speed = Random::GetFloatRange(40.0f, 120.0f); // Mais lento que a explosão
        Vector2 dir(dirX, dirY);
        dir.Normalize(); // Garante consistência na velocidade

        Vector2 particlePos = position + dir * Random::GetFloatRange(0.0f, 5.0f);

        Actor* particle = new Actor(this);
        particle->SetPosition(particlePos);

        DrawComponent* drawComp = new DrawComponent(particle, particleVertices);
        // Variação de cor (tom de poeira/cinza)
        Vector3 particleColor = color;
        particleColor.x = Math::Clamp(particleColor.x + Random::GetFloatRange(-0.1f, 0.1f), 0.0f, 1.0f);
        particleColor.y = Math::Clamp(particleColor.y + Random::GetFloatRange(-0.1f, 0.1f), 0.0f, 1.0f);
        particleColor.z = Math::Clamp(particleColor.z + Random::GetFloatRange(-0.1f, 0.1f), 0.0f, 1.0f);
        drawComp->SetColor(particleColor);
        drawComp->SetFilled(true);
        drawComp->SetUseCamera(true);

        RigidBodyComponent* rbComp = new RigidBodyComponent(particle, 0.1f);
        rbComp->SetVelocity(dir * speed);

        particle->SetLifetime(0.5f); // Longer lifetime for spectacular visibility
    }
}

// NOVO: Círculo de demarcação da explosão
void Game::SpawnExplosionRing(const Vector2& position, float radius)
{
    Actor* ring = new Actor(this);
    ring->SetPosition(position);

    // Vértices do círculo (baseado no Enemy.cpp)
    std::vector<Vector2> vertices;
    const int numVertices = 32; // Mais vértices para um círculo suave
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * radius, Math::Sin(angle) * radius));
    }
    // Adiciona o primeiro vértice no final para fechar o loop
    vertices.emplace_back(vertices[0]);


    DrawComponent* drawComp = new DrawComponent(ring, vertices);
    drawComp->SetColor(Vector3(1.0f, 0.5f, 0.2f)); // Laranja da explosão
    drawComp->SetFilled(false); // Importante: desenha só a linha (o anel)
    drawComp->SetUseCamera(true);

    ring->SetLifetime(0.3f); // <-- Tempo de vida adicionado
}



// =====================================================================================
// NOVO SISTEMA DE SPAWN: regras por tempo/wave + hordas + ganchos de chefe
// =====================================================================================

void Game::InitSpawnRules()
{
    const float INF = std::numeric_limits<float>::infinity();

    mSpawnRules.clear();
    mRuleTimers.clear();
    mTimedHordes.clear();

    // Regras contínuas (por intervalo) - Spectacular Vampire Survivors balance
    mSpawnRules.push_back({ EnemyKind::Comum, 1,    0.0f,  INF, 0.4f,  5 });  // More enemies, faster spawn for intensity
    // Fast enemies from wave 2
    mSpawnRules.push_back({ EnemyKind::Corredor,  2,   8.0f,  INF, 0.7f,  4 });  // Earlier, more frequent
    // Tanks from wave 3
    mSpawnRules.push_back({ EnemyKind::GordoExplosivo,  3,   20.0f,  INF, 2.2f,  7 });  // More per spawn for impact
    // Elites from wave 4 (earlier for challenge)
    mSpawnRules.push_back({ EnemyKind::Atirador, 4,  40.0f,  INF, 4.5f,  4 });  // More frequent, more per spawn
    mRuleTimers.resize(mSpawnRules.size(), 0.0f);

    // Hordas pontuais (burst em tempos específicos) - More intense hordes
    mTimedHordes.push_back({  45.0f, EnemyKind::Comum,         35, 1, false });  // Earlier, bigger
    mTimedHordes.push_back({  90.0f, EnemyKind::Corredor,      30, 2, false });  // Bigger horde
    mTimedHordes.push_back({ 120.0f, EnemyKind::GordoExplosivo, 15, 3, false });  // More explosive enemies
    mTimedHordes.push_back({ 150.0f, EnemyKind::Atirador,       12, 4, false });  // More shooters
    mTimedHordes.push_back({ 180.0f, EnemyKind::Comum,         40, 5, false });  // Mid-game horde
    mTimedHordes.push_back({ 240.0f, EnemyKind::Corredor,      35, 6, false });  // Late-game speed horde
    mTimedHordes.push_back({ 300.0f, EnemyKind::Comum,         60, 7, false });  // Massive late-game horde
}

void Game::SpawnEnemyOfKind(EnemyKind kind, int count)
{
    if (!mPlayer) return;

    Vector2 playerPos = mPlayer->GetPosition();

    for (int i = 0; i < count; ++i)
    {
        // Posição inicial aleatória ao redor do jogador
        Vector2 spawnPos;
        float distance = 350.0f + Random::GetFloatRange(-50.0f, 50.0f);
        float angle = Random::GetFloatRange(0.0f, Math::TwoPi);
        spawnPos.x = playerPos.x + Math::Cos(angle) * distance;
        spawnPos.y = playerPos.y + Math::Sin(angle) * distance;

        // Garante que o inimigo nasce dentro dos limites do mapa
        float radius = 20.0f;
        spawnPos.x = Math::Clamp(spawnPos.x, radius, (float)WORLD_WIDTH - radius);
        spawnPos.y = Math::Clamp(spawnPos.y, radius, (float)WORLD_HEIGHT - radius);

        // Atributos base que escalam com a wave - Spectacular progression
        float baseHealth = 20.0f + mCurrentWave * 4.0f;  // Balanced health scaling (slightly easier early)
        float baseSpeed  = 85.0f + mCurrentWave * 2.5f;  // Moderate speed scaling
        float baseRadius = 12.0f + mCurrentWave * 0.2f;  // Slow size growth

        // Cria inimigo e aplica atributos conforme o tipo
        Enemy* e = new Enemy(this, kind, baseRadius, baseSpeed, baseHealth);

        switch (kind)
        {
            case EnemyKind::Comum:
                e->SetColor(Vector3(0.9f, 0.1f, 0.1f));
                e->SetDamage(8.0f);  // Reduced for better balance
                e->SetExperienceValue(8.0f + mCurrentWave * 0.5f);  // Scales with wave
                break;

            case EnemyKind::Corredor:
                e->SetColor(Vector3(1.0f, 0.5f, 0.2f));
                e->SetSpeed(280.0f);  // Faster for more challenge
                e->SetDamage(5.0f);  // Less damage but faster
                e->SetExperienceValue(10.0f + mCurrentWave * 0.5f);
                break;

            case EnemyKind::GordoExplosivo:
                e->SetColor(Vector3(0.7f, 0.3f, 0.3f));
                e->SetSpeed(55.0f);
                e->SetExplosionDamage(35.0f + mCurrentWave * 2.0f);  // Scales with wave
                e->SetExplosionRadius(160.0f);  // Slightly larger
                e->SetExperienceValue(18.0f + mCurrentWave * 1.0f);
                e->SetExplodesOnDeath(true);
                break;

            case EnemyKind::Atirador:
                e->SetColor(Vector3(0.2f, 0.6f, 1.0f));
                e->SetSpeed(45.0f);
                e->SetProjectileSpeed(550.0f);  // Faster projectiles
                e->SetShootEvery(1.8f);  // Shoots more frequently
                e->SetExperienceValue(15.0f + mCurrentWave * 0.8f);
                e->SetRangedShooter(true, 1.8f);
                break;
        }

        e->SetPosition(spawnPos);
    }
}

void Game::SpawnHorde(EnemyKind kind, int count)
{
    SpawnEnemyOfKind(kind, count);
}

void Game::UpdateWaveSystem(float deltaTime)
{
    mElapsedSeconds += deltaTime;
    mWaveTimer      += deltaTime;

    // Wave avança a cada 30s (mantendo sua lógica original)
    int newWave = 1 + static_cast<int>(mWaveTimer / 30.0f);
    if (newWave > mCurrentWave)
    {
        mCurrentWave = newWave;

        // --- LÓGICA DE SPAWN DO CHEFE (MODIFICADO) ---
        // É uma onda de chefe (múltipla de 5) E ainda não spawnamos nesta wave?
        if (mCurrentWave % 5 == 0 && mCurrentWave > mLastBossWaveSpawned)
        {
            SpawnBoss(mCurrentWave);
            mLastBossWaveSpawned = mCurrentWave;
        }
        // --- FIM DA LÓGICA DO CHEFE ---
    }

    // Limite global de população - Spectacular Vampire Survivors intensity
    const int maxEnemies = 600 + (mCurrentWave * 50);  // Higher cap for epic battles
    if ((int)mEnemies.size() >= maxEnemies) return;

    // --- PAUSA O SPAWN NORMAL SE UM CHEFE ESTIVER ATIVO (MODIFICADO) ---
    if (!mBosses.empty())
    {
        return; // Não spawna inimigos normais durante a luta do chefe
    }
    // -----------------------------------------------------------

    // 1) Regras contínuas (intervalos por tipo)
    for (size_t i = 0; i < mSpawnRules.size(); ++i)
    {
        auto& r = mSpawnRules[i];
        if (mCurrentWave >= r.minWave &&
            mElapsedSeconds >= r.start &&
            mElapsedSeconds <= r.end)
        {
            mRuleTimers[i] += deltaTime;
            while (mRuleTimers[i] >= r.every && (int)mEnemies.size() < maxEnemies)
            {
                int canSpawn = std::min(r.count, maxEnemies - (int)mEnemies.size());
                if (canSpawn > 0)
                    SpawnEnemyOfKind(r.kind, canSpawn);

                mRuleTimers[i] -= r.every;
            }
        }
    }

    // 2) Hordas pontuais (um disparo)
    for (auto& h : mTimedHordes)
    {
        if (!h.fired &&
            mElapsedSeconds >= h.atTime &&
            mCurrentWave >= h.minWave)
        {
            SpawnHorde(h.kind, h.count);
            h.fired = true;
        }
    }

    // 3) Ganchos de chefes (Lógica antiga removida, substituída pela nova acima)
}
// =====================================================================================

// Em Game.cpp

void Game::DrawUI()
{
    if (!mPlayer)
        return;

    // Draw health bar
    float healthPercent = mPlayer->GetHealth() / mPlayer->GetMaxHealth();

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

    // Draw experience bar - improved visual design
    float expPercent = Math::Clamp(mPlayer->GetExperience() / mPlayer->GetExperienceToNextLevel(), 0.0f, 1.0f);
    std::vector<Vector2> expBg;
    expBg.emplace_back(Vector2(20.0f, 50.0f));
    expBg.emplace_back(Vector2(220.0f, 50.0f));
    expBg.emplace_back(Vector2(220.0f, 62.0f));  // Slightly taller
    expBg.emplace_back(Vector2(20.0f, 62.0f));

    Matrix4 expBgMatrix = Matrix4::Identity;
    VertexArray* expBgVA = createVA(expBg);
    Vector3 expBgColor(0.15f, 0.15f, 0.2f);  // Darker, more visible
    mRenderer->Draw(expBgMatrix, expBgVA, expBgColor);
    delete expBgVA;

    std::vector<Vector2> expBar;
    expBar.emplace_back(Vector2(20.0f, 50.0f));
    expBar.emplace_back(Vector2(20.0f + 200.0f * expPercent, 50.0f));
    expBar.emplace_back(Vector2(20.0f + 200.0f * expPercent, 62.0f));
    expBar.emplace_back(Vector2(20.0f, 62.0f));

    Matrix4 expMatrix = Matrix4::Identity;
    VertexArray* expVA = createVA(expBar);
    // Brighter, more vibrant cyan color
    Vector3 expColor(0.1f, 0.9f, 1.0f);
    mRenderer->Draw(expMatrix, expVA, expColor);
    delete expVA;

    // Wave panel
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

    // Labels - improved visibility with better contrast
    TextRenderer::DrawText(mRenderer, "HP",  Vector2(25.0f,  5.0f), 0.85f, Vector3(1.0f, 0.5f, 0.5f));  // Brighter red
    TextRenderer::DrawText(mRenderer, "EXP", Vector2(25.0f, 45.0f), 0.85f, Vector3(0.3f, 1.0f, 1.0f));  // Brighter cyan

    // Wave text - improved styling with pulsing effect
    std::string waveText = "WAVE " + std::to_string(mCurrentWave);
    Uint32 ticks = SDL_GetTicks();
    float wavePulse = 0.9f + 0.1f * Math::Sin(ticks * 0.005f);  // Subtle pulse
    TextRenderer::DrawText(mRenderer, waveText, Vector2(static_cast<float>(WINDOW_WIDTH) - 140.0f, 35.0f), 0.9f * wavePulse, Vector3(1.0f, 0.9f, 0.3f));

    // Level text - more prominent with glow effect
    std::string levelText = "LVL " + std::to_string(mPlayer->GetLevel());
    float levelGlow = 0.95f + 0.05f * Math::Sin(ticks * 0.008f);  // Subtle glow
    TextRenderer::DrawText(mRenderer, levelText, Vector2(250.0f, 25.0f), 0.9f * levelGlow, Vector3(1.0f, 0.95f, 0.2f));
    
    // Time survived display
    int minutes = static_cast<int>(mElapsedSeconds) / 60;
    int seconds = static_cast<int>(mElapsedSeconds) % 60;
    std::string timeText = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    TextRenderer::DrawText(mRenderer, timeText, Vector2(static_cast<float>(WINDOW_WIDTH) - 140.0f, 65.0f), 0.7f, Vector3(0.7f, 0.7f, 0.9f));
    
    // Kill counter
    std::string killText = "KILLS: " + std::to_string(mKills);
    TextRenderer::DrawText(mRenderer, killText, Vector2(static_cast<float>(WINDOW_WIDTH) - 140.0f, 85.0f), 0.7f, Vector3(0.9f, 0.5f, 0.2f));
    
    // Combo display (only show if combo is active)
    if (mCombo > 0)
    {
        std::string comboText = "COMBO x" + std::to_string(mCombo) + "!";
        float comboScale = 1.0f + (mCombo / 50.0f) * 0.3f; // Scale up with combo
        Vector3 comboColor(1.0f, 0.8f + (mCombo / 100.0f) * 0.2f, 0.2f); // Get more yellow with higher combo
        TextRenderer::DrawText(mRenderer, comboText, Vector2(static_cast<float>(WINDOW_WIDTH) / 2.0f - 80.0f, 100.0f), comboScale, comboColor);
        
        // Combo multiplier
        std::string multText = std::to_string((int)(mComboMultiplier * 100.0f)) + "% XP";
        TextRenderer::DrawText(mRenderer, multText, Vector2(static_cast<float>(WINDOW_WIDTH) / 2.0f - 50.0f, 130.0f), 0.8f, Vector3(0.2f, 1.0f, 0.5f));
    }
    
    // Health text - improved visibility
    std::string healthText = std::to_string((int)mPlayer->GetHealth()) + "/" + std::to_string((int)mPlayer->GetMaxHealth());
    TextRenderer::DrawText(mRenderer, healthText, Vector2(230.0f, 5.0f), 0.75f, Vector3(1.0f, 0.4f, 0.4f));
    
    // Stats panel (top right) - only show if player has upgrades
    float statsY = 80.0f;
    if (mPlayer->GetDamageMultiplier() > 1.1f || mPlayer->GetAttackSpeedMultiplier() > 1.1f)
    {
        std::string damageText = "DMG: " + std::to_string((int)(mPlayer->GetDamageMultiplier() * 100.0f)) + "%";
        TextRenderer::DrawText(mRenderer, damageText, Vector2(static_cast<float>(WINDOW_WIDTH) - 200.0f, statsY), 0.7f, Vector3(1.0f, 0.3f, 0.3f));
        statsY += 20.0f;
        
        std::string speedText = "SPD: " + std::to_string((int)(mPlayer->GetAttackSpeedMultiplier() * 100.0f)) + "%";
        TextRenderer::DrawText(mRenderer, speedText, Vector2(static_cast<float>(WINDOW_WIDTH) - 200.0f, statsY), 0.7f, Vector3(0.3f, 1.0f, 0.3f));
        statsY += 20.0f;
    }
    
    if (mPlayer->GetCritChance() > 0.0f)
    {
        std::string critText = "CRIT: " + std::to_string((int)(mPlayer->GetCritChance() * 100.0f)) + "%";
        TextRenderer::DrawText(mRenderer, critText, Vector2(static_cast<float>(WINDOW_WIDTH) - 200.0f, statsY), 0.7f, Vector3(1.0f, 0.8f, 0.0f));
        statsY += 20.0f;
    }
    
    if (mPlayer->HasLifesteal())
    {
        std::string lifestealText = "LIFESTEAL: " + std::to_string((int)(mPlayer->GetLifestealPercent() * 100.0f)) + "%";
        TextRenderer::DrawText(mRenderer, lifestealText, Vector2(static_cast<float>(WINDOW_WIDTH) - 200.0f, statsY), 0.7f, Vector3(1.0f, 0.0f, 0.5f));
        statsY += 20.0f;
    }
    
    // Active weapon modes
    if (mPlayer->GetProjectilePierce() > 0)
    {
        std::string pierceText = "PIERCE: " + std::to_string(mPlayer->GetProjectilePierce());
        TextRenderer::DrawText(mRenderer, pierceText, Vector2(static_cast<float>(WINDOW_WIDTH) - 200.0f, statsY), 0.6f, Vector3(0.5f, 0.5f, 1.0f));
        statsY += 18.0f;
    }

    // --- ⬇️ BLOCO ADICIONADO PARA A BARRA DE VIDA DO CHEFE ⬇️ ---

    // ---------------------------------
    //  BARRA DE VIDA DO CHEFE (BOSS)
    // ---------------------------------
    if (!mBosses.empty()) // Só desenha se houver um chefe na lista
    {
        // Pega o primeiro chefe da lista (normalmente só haverá um)
        Boss* boss = mBosses[0];
        if (boss)
        {
            float healthPercent = boss->GetHealth() / boss->GetMaxHealth();
            if (healthPercent < 0.0f) healthPercent = 0.0f;

            // Posição e tamanho da barra (centralizada no topo)
            const float barWidth = 600.0f; // Barra larga
            const float barHeight = 20.0f;
            // Centraliza horizontalmente
            const float barX = (static_cast<float>(WINDOW_WIDTH) / 2.0f) - (barWidth / 2.0f);
            const float barY = 40.0f; // Posição Y (perto do topo)

            // 1. Desenha o fundo da barra (cinza escuro)
            std::vector<Vector2> bossBg;
            bossBg.emplace_back(Vector2(barX, barY));
            bossBg.emplace_back(Vector2(barX + barWidth, barY));
            bossBg.emplace_back(Vector2(barX + barWidth, barY + barHeight));
            bossBg.emplace_back(Vector2(barX, barY + barHeight));

            Matrix4 bossBgMatrix = Matrix4::Identity;
            VertexArray* bossBgVA = createVA(bossBg);
            Vector3 bossBgColor(0.2f, 0.2f, 0.2f); // Cinza escuro
            mRenderer->Draw(bossBgMatrix, bossBgVA, bossBgColor);
            delete bossBgVA;

            // 2. Desenha a barra de vida (vermelha)
            std::vector<Vector2> bossHealth;
            bossHealth.emplace_back(Vector2(barX, barY));
            bossHealth.emplace_back(Vector2(barX + barWidth * healthPercent, barY));
            bossHealth.emplace_back(Vector2(barX + barWidth * healthPercent, barY + barHeight));
            bossHealth.emplace_back(Vector2(barX, barY + barHeight));

            Matrix4 bossHealthMatrix = Matrix4::Identity;
            VertexArray* bossHealthVA = createVA(bossHealth);
            // Cor (vermelho-roxo, para combinar com o chefe)
            Vector3 bossHealthColor(0.9f, 0.1f, 0.5f);
            mRenderer->Draw(bossHealthMatrix, bossHealthVA, bossHealthColor);
            delete bossHealthVA;

            // 3. Desenha o rótulo "BOSS"
            // Centraliza o texto acima da barra
            float labelX = (static_cast<float>(WINDOW_WIDTH) / 2.0f) - 30.0f; // Ajuste baseado na sua fonte
            TextRenderer::DrawText(mRenderer, "BOSS", Vector2(labelX, 15.0f), 1.0f, Vector3(1.0f, 0.2f, 0.2f));
        }
    }
    // --- ⬆️ FIM DO BLOCO ADICIONADO ⬆️ ---

    if (mPlayer && mPlayer->GetPendingUpgrades() > 0)
    {
        std::string upgradeText = "LEVEL UP! PRESS [U] (" + std::to_string(mPlayer->GetPendingUpgrades()) + ")";
        
        if (static_cast<int>(SDL_GetTicks() / 500) % 2 == 0)
        {
            TextRenderer::DrawText(mRenderer, upgradeText, Vector2(WINDOW_WIDTH / 2.0f - 180.0f, 100.0f), 1.2f, Vector3(1.0f, 1.0f, 0.0f));
        }
        else
        {
            TextRenderer::DrawText(mRenderer, upgradeText, Vector2(WINDOW_WIDTH / 2.0f - 180.0f, 100.0f), 1.2f, Vector3(1.0f, 1.0f, 1.0f));
        }
    }
}

void Game::CleanupGame()
{
    // Set player to nullptr FIRST before deleting actors
    // This prevents any code from accessing the player after it's deleted
    mPlayer = nullptr;
    
    // Clear all actor reference lists BEFORE deleting to prevent destructors from accessing cleared vectors
    // The actual actors will be deleted via mActors
    mEnemies.clear();
    mProjectiles.clear();
    mBosses.clear();
    mExperienceOrbs.clear();
    mDeferredExperience.clear();
    
    // Clear drawables before deleting actors (their destructors try to remove themselves)
    mDrawables.clear();
    
    // Remove all actors (this will delete them)
    // Their destructors might try to remove themselves from vectors, but we've already cleared them
    while (!mActors.empty())
    {
        Actor* actor = mActors.back();
        mActors.pop_back();
        delete actor;
    }
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
    if (!actor) return;  // Safety check
    
    // Don't remove if we're currently updating actors (to avoid iterator invalidation)
    // The UpdateActors function handles removal itself
    if (mUpdatingActors)
    {
        return;
    }
    
    // Remove from pending actors first
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }

    // Remove from main actors list
    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
    
    // Note: It's safe if the actor isn't found - this can happen if it was
    // already removed manually before deletion (which we do in UpdateActors)
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
    if (!drawable) return;
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
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }
        DrawUI();
        mPauseMenu->Draw(mRenderer);
    }
    else if (mGameState == MenuState::UpgradeMenu)
    {
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }
        DrawUI();
        mUpgradeMenu->Draw(mRenderer);
    }
    else if (mGameState == MenuState::Playing || mGameState == MenuState::GameOver)
    {
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }

        DrawUI();

        if (mGameState == MenuState::GameOver)
        {
            // Improved game over screen
            std::string gameOverText = "GAME OVER";
            std::string waveText = "Wave: " + std::to_string(mCurrentWave);
            std::string levelText = "Level: " + std::to_string(mPlayer ? mPlayer->GetLevel() : 0);
            std::string restartText = "Press SPACE to restart";
            
            TextRenderer::DrawText(mRenderer, gameOverText, Vector2(WINDOW_WIDTH / 2.0f - 180.0f, WINDOW_HEIGHT / 2.0f - 80.0f), 2.2f, Vector3(0.95f, 0.1f, 0.1f));
            TextRenderer::DrawText(mRenderer, waveText, Vector2(WINDOW_WIDTH / 2.0f - 80.0f, WINDOW_HEIGHT / 2.0f - 20.0f), 1.2f, Vector3(1.0f, 0.9f, 0.3f));
            TextRenderer::DrawText(mRenderer, levelText, Vector2(WINDOW_WIDTH / 2.0f - 80.0f, WINDOW_HEIGHT / 2.0f + 10.0f), 1.2f, Vector3(1.0f, 0.9f, 0.3f));
            TextRenderer::DrawText(mRenderer, restartText, Vector2(WINDOW_WIDTH / 2.0f - 140.0f, WINDOW_HEIGHT / 2.0f + 50.0f), 1.1f, Vector3(0.9f, 0.9f, 0.9f));
        }
        else if (mIsDebugging)
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

    TextRenderer::Shutdown();
    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}