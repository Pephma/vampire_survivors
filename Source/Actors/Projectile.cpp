#include "Projectile.h"
#include "../Game.h"
#include "../Actors/Enemy.h"
#include "../Actors/Player.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Math.h"

Projectile::Projectile(class Game* game,
                       const Vector2& position,
                       const Vector2& direction,
                       float speed,
                       bool fromPlayer,
                       float damage)
    : Actor(game)
    , mDirection(direction)
    , mSpeed(speed)
    , mLifetime(2.0f)
    , mFromPlayer(fromPlayer)
    , mDamage(damage)
{
    SetPosition(position);

    // Create small circular projectile
    std::vector<Vector2> vertices;
    float radius = 5.0f;
    int numVertices = 12;
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * radius, Math::Sin(angle) * radius));
    }

    mDrawComponent = new DrawComponent(this, vertices);

    // Cor diferente por origem (feedback visual)
    Vector3 projectileColor = mFromPlayer
        ? Vector3(0.2f, 0.9f, 1.0f)   // player: ciano
        : Vector3(1.0f, 0.5f, 0.2f);  // inimigo: laranja
    mDrawComponent->SetColor(projectileColor);
    mDrawComponent->SetFilled(true);

    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);

    mRigidBodyComponent->SetVelocity(mDirection * mSpeed);

    game->AddProjectile(this);
}

Projectile::~Projectile()
{
    GetGame()->RemoveProjectile(this);
}

void Projectile::OnUpdate(float deltaTime)
{
    // vida do projétil
    mLifetime -= deltaTime;
    if (mLifetime <= 0.0f)
    {
        GetGame()->RemoveProjectile(this);
        SetState(ActorState::Destroy);
        return;
    }

    // colisão
    if (mFromPlayer)
    {
        // projétil do jogador acerta inimigos
        for (auto* e : GetGame()->GetEnemies())
        {
            auto* enemyCol = e->GetComponent<CircleColliderComponent>();
            if (!enemyCol) continue;

            if (mCircleColliderComponent->Intersect(*enemyCol))
            {
                e->TakeDamage(mDamage);

                GetGame()->RemoveProjectile(this);
                SetState(ActorState::Destroy);
                return;
            }
        }
    }
    else
    {
        // projétil inimigo acerta o jogador
        auto* p = GetGame()->GetPlayer();
        if (p)
        {
            auto* playerCol = p->GetComponent<CircleColliderComponent>();
            if (playerCol && mCircleColliderComponent->Intersect(*playerCol))
            {
                p->TakeDamage(mDamage);

                GetGame()->RemoveProjectile(this);
                SetState(ActorState::Destroy);
                return;
            }
        }
    }

    // remover se sair dos limites do mundo
    Vector2 pos = GetPosition();
    if (pos.x < -100 || pos.x > Game::WORLD_WIDTH + 100 ||
        pos.y < -100 || pos.y > Game::WORLD_HEIGHT + 100)
    {
        GetGame()->RemoveProjectile(this);
        SetState(ActorState::Destroy);
        return;
    }
}
