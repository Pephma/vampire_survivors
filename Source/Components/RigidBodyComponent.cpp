//
// Created by Lucas N. Ferreira on 08/09/23.
//

#include <SDL.h>
#include "RigidBodyComponent.h"
#include "../Actors/Actor.h"
#include "../Game.h"
#include "../Random.h"

RigidBodyComponent::RigidBodyComponent(class Actor* owner, float mass, int updateOrder)
        :Component(owner, updateOrder)
        ,mMass(mass)
        ,mAngularSpeed(0.0f)
        ,mVelocity(Vector2::Zero)
        ,mAcceleration(Vector2::Zero)
{

}

void RigidBodyComponent::ApplyForce(const Vector2 &force)
{

    mAcceleration += force * (1.0f / mMass);
}

void RigidBodyComponent::Update(float deltaTime)
{

    mVelocity += mAcceleration * deltaTime;
    

    if (mVelocity.Length() > MAX_VELOCITY)
    {
        mVelocity.Normalize();
        mVelocity *= MAX_VELOCITY;
    }
    

    Vector2 position = mOwner->GetPosition();
    position += mVelocity * deltaTime;
    
 
    ScreenWrap(position);
    mOwner->SetPosition(position);
    
    
    mAcceleration = Vector2::Zero;
    
   
    if (Math::Abs(mVelocity.x) < 0.01f)
    {
        mVelocity.x = 0.0f;
    }
    if (Math::Abs(mVelocity.y) < 0.01f)
    {
        mVelocity.y = 0.0f;
    }
    
  
    float rotation = mOwner->GetRotation();
    rotation += mAngularSpeed * deltaTime;
    mOwner->SetRotation(rotation);
}

void RigidBodyComponent::ScreenWrap(Vector2 &position)
{
    // For players and enemies, clamp to world bounds instead of wrapping
    // Only projectiles can go out of bounds
}
