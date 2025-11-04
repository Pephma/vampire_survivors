

#include "ParticleSystemComponent.h"
#include "../Game.h"
#include "../Actors/Enemy.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"

Particle::Particle(class Game* game, std::vector<Vector2> &vertices)
    : Actor(game)
    , mDrawComponent(nullptr)
    , mRigidBodyComponent(nullptr)
    , mCircleColliderComponent(nullptr)
    , mIsDead(true)
    , mLifeTime(1.0f)
{
    
    mDrawComponent = new DrawComponent(this, vertices);
    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f); 
    mCircleColliderComponent = new CircleColliderComponent(this, 2.0f); 

    SetState(ActorState::Paused);
    mDrawComponent->SetVisible(false);
    mIsDead = true;
}

void Particle::Kill()
{
  
    mIsDead = true;
    SetState(ActorState::Paused);
    mDrawComponent->SetVisible(false);

    mRigidBodyComponent->SetVelocity(Vector2::Zero);
}

void Particle::Awake(const Vector2 &position, float rotation, float lifetime)
{
 
    mLifeTime = lifetime;
    mIsDead = false;
    
 
    SetState(ActorState::Active);
    mDrawComponent->SetVisible(true);
 
    SetPosition(position);
    SetRotation(rotation);
}

void Particle::OnUpdate(float deltaTime)
{
    
    mLifeTime -= deltaTime;

    if (mLifeTime <= 0.0f)
    {
        Kill();
        return;
    }
    
    // Check collision with enemies (optional - projectiles handle this in Enemy.cpp)
    // This is kept for compatibility but can be removed if not needed
    for (auto enemy : GetGame()->GetEnemies())
    {
        if (mCircleColliderComponent->Intersect(*enemy->GetComponent<CircleColliderComponent>()))
        {
            // Particle hit enemy - kill the particle
            Kill();
            break;
        }
    }
}

ParticleSystemComponent::ParticleSystemComponent(class Actor* owner, std::vector<Vector2> &vertices, int poolSize, int updateOrder)
    : Component(owner, updateOrder)
{
   
    for (int i = 0; i < poolSize; i++)
    {
        Particle* particle = new Particle(owner->GetGame(), vertices);
        mParticles.emplace_back(particle);
    }
}

void ParticleSystemComponent::EmitParticle(float lifetime, float speed, const Vector2& offsetPosition)
{
   
    for (auto particle : mParticles)
    {
        if (particle->IsDead())
        {
           
            Vector2 spawnPosition = mOwner->GetPosition() + offsetPosition;
            
            
            particle->Awake(spawnPosition, mOwner->GetRotation(), lifetime);
            
            
            Vector2 forward = mOwner->GetForward();
            particle->GetComponent<RigidBodyComponent>()->ApplyForce(forward * speed);
            
         
            break;
        }
    }
}