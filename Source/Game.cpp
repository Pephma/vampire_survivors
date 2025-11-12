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

        // (Opcional) limpeza de partículas longe da câmera
        Vector2 actorPos = actor->GetPosition();
        Vector2 cameraPos = mCameraPosition;
        float distance = Vector2::Distance(actorPos, cameraPos);
        if (distance > 1000.0f && actor->GetState() == ActorState::Active)
        {
            // heurística de limpeza — mantido como no original
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
                           float damage)
{
    Projectile* projectile = new Projectile(this, position, direction, speed, fromPlayer, damage);
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

    // --- ADICIONE ESTAS DUAS LINHAS AQUI ---
    // Spawna o chefe imediatamente (Wave 1, Segundo 1)
    SpawnBoss(10);
    // Avisa o sistema que o chefe da wave 1 já foi
    mLastBossWaveSpawned = 10;
    // ----------------------------------------
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

    // follow
    Vector2 targetCameraPos = playerPos;

    // Smooth camera movement
    Vector2 diff = targetCameraPos - mCameraPosition;
    mCameraPosition += diff * (deltaTime * 5.0f); // 5.0f = follow speed

    // Screen shake
    if (mScreenShakeDuration > 0.0f)
    {
        mScreenShakeDuration -= deltaTime;
        if (mScreenShakeDuration <= 0.0f)
        {
            mScreenShakeAmount = 0.0f;
            mScreenShakeDuration = 0.0f;
        }

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

// RENOMEADO: Esta função agora é específica para a explosão
void Game::SpawnExplosionParticles(const Vector2& position, const Vector3& color)
{
    CreateDeathParticles(position, color, 16);
}

void Game::CreateDeathParticles(const Vector2& position, const Vector3& color, int count)
{
    // Create small particles for death effect
    std::vector<Vector2> particleVertices;
    float size = Random::GetFloatRange(1.5f, 3.0f);
    particleVertices.emplace_back(Vector2(-size, -size));
    particleVertices.emplace_back(Vector2(size, -size));
    particleVertices.emplace_back(Vector2(size, size));
    particleVertices.emplace_back(Vector2(-size, size));

    for (int i = 0; i < count; ++i)
    {
        float angle = Random::GetFloatRange(0.0f, Math::TwoPi);
        float speed = Random::GetFloatRange(80.0f, 250.0f);
        Vector2 dir(Math::Cos(angle), Math::Sin(angle));
        Vector2 particlePos = position + dir * Random::GetFloatRange(0.0f, 5.0f);

        Actor* particle = new Actor(this);
        particle->SetPosition(particlePos);

        DrawComponent* drawComp = new DrawComponent(particle, particleVertices);
        Vector3 particleColor = color;
        particleColor.x = Math::Clamp(particleColor.x + Random::GetFloatRange(-0.2f, 0.2f), 0.0f, 1.0f);
        particleColor.y = Math::Clamp(particleColor.y + Random::GetFloatRange(-0.2f, 0.2f), 0.0f, 1.0f);
        particleColor.z = Math::Clamp(particleColor.z + Random::GetFloatRange(-0.2f, 0.2f), 0.0f, 1.0f);
        drawComp->SetColor(particleColor);
        drawComp->SetFilled(true);
        drawComp->SetUseCamera(true);

        RigidBodyComponent* rbComp = new RigidBodyComponent(particle, 0.1f);
        rbComp->SetVelocity(dir * speed);

        particle->SetState(ActorState::Active);
        particle->SetLifetime(0.5f); // <-- Tempo de vida adicionado
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

    int count = 12; // Menos partículas que a explosão

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

        particle->SetLifetime(0.2f); // <-- Tempo de vida adicionado
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

    // Regras contínuas (por intervalo)
    mSpawnRules.push_back({ EnemyKind::Comum, 1,    0.0f,  INFINITY, 0.70f,  2 });
    // Fast entram a partir da wave 3
    mSpawnRules.push_back({ EnemyKind::Corredor,  3,   20.0f,  INFINITY, 1.20f,  3 });
    // Tanks a partir da wave 5
    mSpawnRules.push_back({ EnemyKind::GordoExplosivo,  1,   0.0f,  INFINITY, 2.50f,  10 });
    // Elites espaçados a partir da wave 7
    mSpawnRules.push_back({ EnemyKind::Atirador, 7,  120.0f,  INFINITY, 8.00f,  1 });
    mRuleTimers.resize(mSpawnRules.size(), 0.0f);

    // Hordas pontuais (burst em tempos específicos)
    mTimedHordes.push_back({  75.0f, EnemyKind::Comum,         25, 1, false });
    mTimedHordes.push_back({ 135.0f, EnemyKind::Corredor,      18, 2, false });
    mTimedHordes.push_back({ 180.0f, EnemyKind::GordoExplosivo, 8, 3, false });
    mTimedHordes.push_back({ 240.0f, EnemyKind::Atirador,       6, 4, false });
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

        // Atributos base que escalam com a wave
        float baseHealth = 25.0f + mCurrentWave * 8.0f;
        float baseSpeed  = 70.0f + mCurrentWave * 3.0f;
        float baseRadius = 12.0f + mCurrentWave * 0.5f;

        // Cria inimigo e aplica atributos conforme o tipo
        Enemy* e = new Enemy(this, baseRadius, baseSpeed, baseHealth);

        switch (kind)
        {
            case EnemyKind::Comum:
                e->SetColor(Vector3(0.9f, 0.1f, 0.1f));
                e->SetDamage(10.0f);
                e->SetExperienceValue(10.0f);
                break;

            case EnemyKind::Corredor:
                e->SetColor(Vector3(1.0f, 0.5f, 0.2f));
                e->SetSpeed(250.0f);
                e->SetDamage(6.0f);
                e->SetExperienceValue(12.0f);
                break;

            case EnemyKind::GordoExplosivo:
                e->SetColor(Vector3(0.7f, 0.3f, 0.3f));
                e->SetSpeed(60.0f);
                e->SetExplosionDamage(40.0f);
                e->SetExplosionRadius(150.0f);
                e->SetExperienceValue(20.0f);
                e->SetExplodesOnDeath(true);
                break;

            case EnemyKind::Atirador:
                e->SetColor(Vector3(0.2f, 0.6f, 1.0f));
                e->SetSpeed(50.0f);
                e->SetProjectileSpeed(500.0f);
                e->SetShootEvery(2.0f);
                e->SetExperienceValue(15.0f);
                e->SetRangedShooter(true, 2.0f);
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

    // Limite global de população para não saturar
    const int maxEnemies = 400 + (mCurrentWave * 30);
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

    // Labels
    TextRenderer::DrawText(mRenderer, "HP",  Vector2(25.0f,  5.0f), 0.7f, Vector3(1.0f, 0.5f, 0.5f));
    TextRenderer::DrawText(mRenderer, "EXP", Vector2(25.0f, 45.0f), 0.7f, Vector3(0.5f, 0.8f, 1.0f));

    // Wave text
    std::string waveText = "WAVE " + std::to_string(mCurrentWave);
    TextRenderer::DrawText(mRenderer, waveText, Vector2(static_cast<float>(WINDOW_WIDTH) - 140.0f, 35.0f), 0.8f, Vector3(1.0f, 1.0f, 1.0f));

    // Level text
    std::string levelText = "LVL " + std::to_string(mPlayer->GetLevel());
    TextRenderer::DrawText(mRenderer, levelText, Vector2(250.0f, 25.0f), 0.8f, Vector3(1.0f, 1.0f, 0.0f));

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

    // --- ADICIONADO PARA O CHEFE ---
    mBosses.clear();
    // -----------------------------
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
    else if (mGameState == MenuState::Playing)
    {
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
        for (auto drawable : mDrawables)
        {
            drawable->Draw(mRenderer);
        }
        DrawUI();

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