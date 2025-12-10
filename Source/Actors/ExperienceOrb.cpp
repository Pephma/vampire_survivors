#include "ExperienceOrb.h"
#include "../Game.h"
#include "../Actors/Player.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/AnimatorComponent.h"
#include "../Math.h"
#include "../Random.h"

ExperienceOrb::ExperienceOrb(class Game* game, const Vector2& position, float experienceValue)
    : Actor(game)
    ,     mExperienceValue(experienceValue)
    , mLifetime(20.0f)  // Orbs last 20 seconds before disappearing
    , mFloatPhase(Random::GetFloatRange(0.0f, Math::TwoPi))
    , mCollected(false)
{
    SetPosition(position);
    SetScale(Vector2(1.0f, -1.0f));
    
    mAnimatorComponent = new AnimatorComponent(this,
        "../Assets/Sprites/Xp/xp.png",
        "../Assets/Sprites/Xp/xp.json",
        16,  
        16   
    );
    
    mAnimatorComponent->AddAnimation("idle", {0});
    mAnimatorComponent->SetAnimation("idle");
    mAnimatorComponent->SetAnimFPS(0.0f);

    float radius = 8.0f;
    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);
    
    // Give it a small random velocity
    float angle = Random::GetFloatRange(0.0f, Math::TwoPi);
    float speed = Random::GetFloatRange(20.0f, 60.0f);
    mRigidBodyComponent->SetVelocity(Vector2(Math::Cos(angle) * speed, Math::Sin(angle) * speed));
}

ExperienceOrb::~ExperienceOrb()
{
    // Don't remove from list in destructor - it's already removed before deletion
    // Removing here can cause crashes if called during iteration or after cleanup
    // The Game class handles removal explicitly before deletion
    
    // Clear component pointers to prevent any dangling references
    // Components are deleted by Actor destructor, but clear pointers just in case
    mAnimatorComponent = nullptr;
    mCircleColliderComponent = nullptr;
    mRigidBodyComponent = nullptr;
}

void ExperienceOrb::OnUpdate(float deltaTime)
{
    // Don't update if already collected or destroyed
    if (mCollected || GetState() == ActorState::Destroy || GetState() == ActorState::Paused)
    {
        return;
    }
    
    // Get game pointer ONCE at the start - store it for the entire function
    Game* game = GetGame();
    if (!game)
    {
        SetState(ActorState::Destroy);
        return;
    }
    
    // Don't update if game is not in Playing state (e.g., upgrade menu is showing)
    if (game->GetState() != MenuState::Playing)
    {
        return;
    }
    
    // Safety check - don't proceed if components are invalid
    if (!mCircleColliderComponent || !mRigidBodyComponent)
    {
        SetState(ActorState::Destroy);
        return;
    }
    
    mLifetime -= deltaTime;
    if (mLifetime <= 0.0f)
    {
        // Mark for destruction - Game::UpdateActors will handle removal from list
        SetState(ActorState::Destroy);
        return;
    }
    
    // Floating animation
    mFloatPhase += deltaTime * 2.0f;
    if (mFloatPhase > Math::TwoPi) mFloatPhase -= Math::TwoPi;
    
    Vector2 pos = GetPosition();
    pos.y += Math::Sin(mFloatPhase) * 0.5f;  // Gentle floating
    SetPosition(pos);
    
    // Check for player collection - add null checks everywhere
    
    auto* player = game->GetPlayer();
    if (!player || player->GetState() != ActorState::Active || player->GetHealth() <= 0.0f)
    {
        // Player not available or dead, skip collection check
        // But continue with magnet effect if player exists
        if (player && mRigidBodyComponent)
        {
            // Magnet effect - pull toward player if close
            Vector2 toPlayer = player->GetPosition() - GetPosition();
            float distance = toPlayer.Length();
            if (distance < 250.0f && distance > 5.0f)  // Increased range
            {
                toPlayer.Normalize();
                float pullStrength = (250.0f - distance) / 250.0f;
                Vector2 pullForce = toPlayer * pullStrength * 800.0f * deltaTime;  // Much stronger pull (was 500.0f)
                mRigidBodyComponent->SetVelocity(mRigidBodyComponent->GetVelocity() + pullForce);
                
                Vector2 vel = mRigidBodyComponent->GetVelocity();
                if (vel.Length() > 450.0f)  // Higher max speed (was 300.0f)
                {
                    vel.Normalize();
                    mRigidBodyComponent->SetVelocity(vel * 450.0f);
                }
            }
        }
        return;
    }
    
    auto* playerCollider = player->GetComponent<CircleColliderComponent>();
    if (playerCollider && mCircleColliderComponent && mCircleColliderComponent->Intersect(*playerCollider))
    {
        // CRITICAL: Check if already collected to prevent double collection
        if (mCollected) return;
        
        // Collect the orb - mark as collected IMMEDIATELY to prevent double collection
        mCollected = true;
        
        // Store ALL values we need BEFORE any state changes or component access
        Vector2 orbPosition = GetPosition();
        float expValue = mExperienceValue;
        
        // CRITICAL: Add deferred experience BEFORE marking as Destroy
        // This ensures the experience is queued before any cleanup happens
        if (game)
        {
            Game::DeferredExperience deferred;
            deferred.amount = expValue;
            deferred.position = orbPosition;
            game->AddDeferredExperience(deferred);
        }
        
        // Mark for destruction LAST - this prevents any further updates
        // Game::UpdateActors will safely handle removal from list before deletion
        SetState(ActorState::Destroy);
        
        return;  // Exit immediately - don't do anything else
    }
    
    // Magnet effect - pull toward player if close (increased range and strength)
    if (mRigidBodyComponent)
    {
        Vector2 toPlayer = player->GetPosition() - GetPosition();
        float distance = toPlayer.Length();
        if (distance < 250.0f && distance > 5.0f)  // Increased range for better collection
        {
            toPlayer.Normalize();
            float pullStrength = (250.0f - distance) / 250.0f;  // Stronger when closer
            Vector2 pullForce = toPlayer * pullStrength * 800.0f * deltaTime;  // Much stronger pull (was 500.0f)
            mRigidBodyComponent->SetVelocity(mRigidBodyComponent->GetVelocity() + pullForce);
            
            // Clamp max speed - increased for snappier collection
            Vector2 vel = mRigidBodyComponent->GetVelocity();
            if (vel.Length() > 450.0f)  // Higher max speed for faster collection (was 300.0f)
            {
                vel.Normalize();
                mRigidBodyComponent->SetVelocity(vel * 450.0f);
            }
        }
        
        // Friction to slow down over time
        Vector2 vel = mRigidBodyComponent->GetVelocity();
        vel *= (1.0f - deltaTime * 2.0f);  // Decay velocity
        if (vel.Length() < 5.0f)
        {
            vel = Vector2::Zero;
        }
        mRigidBodyComponent->SetVelocity(vel);
    }
}

