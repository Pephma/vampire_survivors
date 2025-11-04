#include "Projectile.h"
#include "../Game.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Math.h"

Projectile::Projectile(class Game* game, const Vector2& position, const Vector2& direction, float speed)
    : Actor(game)
    , mDirection(direction)
    , mSpeed(speed)
    , mLifetime(2.0f)
{
    SetPosition(position);
    
    // Create small circular projectile
    std::vector<Vector2> vertices;
    float radius = 5.0f; // Bigger projectiles for more impact
    int numVertices = 12; // More vertices for smoother circle
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * radius, Math::Sin(angle) * radius));
    }
    
    mDrawComponent = new DrawComponent(this, vertices);
    // Cooler projectile colors - bright cyan/blue with glow effect
    Vector3 projectileColor = Vector3(0.2f, 0.9f, 1.0f); // Bright cyan-blue
    mDrawComponent->SetColor(projectileColor);
    mDrawComponent->SetFilled(true); // Filled for better visibility
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
    mLifetime -= deltaTime;
    if (mLifetime <= 0.0f)
    {
        GetGame()->RemoveProjectile(this);
        SetState(ActorState::Destroy);
        return;
    }
    
    // Remove if out of world bounds
    Vector2 pos = GetPosition();
    if (pos.x < -100 || pos.x > Game::WORLD_WIDTH + 100 ||
        pos.y < -100 || pos.y > Game::WORLD_HEIGHT + 100)
    {
        GetGame()->RemoveProjectile(this);
        SetState(ActorState::Destroy);
    }
}

