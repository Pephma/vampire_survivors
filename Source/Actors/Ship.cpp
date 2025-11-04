
#include "Ship.h"
#include "Asteroid.h"
#include "../Game.h"
#include "../Random.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/ParticleSystemComponent.h"

Ship::Ship(Game* game,
           const float height,
           const float forwardForce,
           const float rotationForce)
        : Actor(game)
        , mLaserCooldown(0.f)
        , mHeight(height)
        , mRigidBodyComponent(nullptr)
        , mDrawComponent(nullptr)
        , mThrusterComponent(nullptr)
        , mForwardSpeed(forwardForce)
        , mRotationForce(rotationForce)
        , mThrusterFlashTimer(0.0f)
        , mIsThrusting(false)
{

    std::vector<Vector2> vertices;
    

    vertices.emplace_back(Vector2(0.0f, mHeight / 2.0f));              
    vertices.emplace_back(Vector2(-mHeight / 10.0f, mHeight / 2.5f));  
    vertices.emplace_back(Vector2(-mHeight / 8.0f, mHeight / 3.0f));   
    vertices.emplace_back(Vector2(-mHeight / 6.0f, mHeight / 5.0f));  

    vertices.emplace_back(Vector2(-mHeight / 2.0f, mHeight / 6.0f));  
    vertices.emplace_back(Vector2(-mHeight / 2.3f, 0.0f));            
    vertices.emplace_back(Vector2(-mHeight / 2.0f, -mHeight / 6.0f));  
    vertices.emplace_back(Vector2(-mHeight / 4.0f, -mHeight / 3.0f));  

    vertices.emplace_back(Vector2(-mHeight / 8.0f, -mHeight / 2.2f));  
    vertices.emplace_back(Vector2(-mHeight / 12.0f, -mHeight / 2.0f)); 
    vertices.emplace_back(Vector2(-mHeight / 16.0f, -mHeight / 2.5f)); 
    vertices.emplace_back(Vector2(0.0f, -mHeight / 2.8f));         
    vertices.emplace_back(Vector2(mHeight / 16.0f, -mHeight / 2.5f));  
    vertices.emplace_back(Vector2(mHeight / 12.0f, -mHeight / 2.0f)); 
    vertices.emplace_back(Vector2(mHeight / 8.0f, -mHeight / 2.2f));  

    vertices.emplace_back(Vector2(mHeight / 4.0f, -mHeight / 3.0f));  
    vertices.emplace_back(Vector2(mHeight / 2.0f, -mHeight / 6.0f));  
    vertices.emplace_back(Vector2(mHeight / 2.3f, 0.0f));             
    vertices.emplace_back(Vector2(mHeight / 2.0f, mHeight / 6.0f));    
    vertices.emplace_back(Vector2(mHeight / 6.0f, mHeight / 5.0f));   
    vertices.emplace_back(Vector2(mHeight / 8.0f, mHeight / 3.0f));    
    vertices.emplace_back(Vector2(mHeight / 10.0f, mHeight / 2.5f));   

    mDrawComponent = new DrawComponent(this, vertices);
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, mHeight / 2.5f);

    std::vector<Vector2> thrusterVertices;

    thrusterVertices.emplace_back(Vector2(-mHeight / 12.0f, -mHeight / 2.0f)); 
    thrusterVertices.emplace_back(Vector2(-mHeight / 16.0f, -mHeight * 0.8f));  
    thrusterVertices.emplace_back(Vector2(-mHeight / 20.0f, -mHeight * 0.75f)); 

    thrusterVertices.emplace_back(Vector2(0.0f, -mHeight * 0.6f));         

    thrusterVertices.emplace_back(Vector2(mHeight / 20.0f, -mHeight * 0.75f));  
    thrusterVertices.emplace_back(Vector2(mHeight / 16.0f, -mHeight * 0.8f)); 
    thrusterVertices.emplace_back(Vector2(mHeight / 12.0f, -mHeight / 2.0f)); 

    mThrusterComponent = new DrawComponent(this, thrusterVertices, 99);
    mThrusterComponent->SetVisible(false);

                    
    std::vector<Vector2> laserVertices;
    laserVertices.emplace_back(Vector2(0.0f, 12.0f));       
    laserVertices.emplace_back(Vector2(-1.5f, 8.0f));      
    laserVertices.emplace_back(Vector2(-3.0f, 2.0f));       
    laserVertices.emplace_back(Vector2(-2.5f, -2.0f));     
    laserVertices.emplace_back(Vector2(-1.0f, -8.0f));     
    laserVertices.emplace_back(Vector2(0.0f, -6.0f));      
    laserVertices.emplace_back(Vector2(1.0f, -8.0f));       
    laserVertices.emplace_back(Vector2(2.5f, -2.0f));       
    laserVertices.emplace_back(Vector2(3.0f, 2.0f));       
    laserVertices.emplace_back(Vector2(1.5f, 8.0f));       

    mWeapon = new ParticleSystemComponent(this, laserVertices, 25);


    std::vector<Vector2> thrusterParticleVertices;
    thrusterParticleVertices.emplace_back(Vector2(0.0f, 2.0f));    
    thrusterParticleVertices.emplace_back(Vector2(-0.6f, 0.6f));    
    thrusterParticleVertices.emplace_back(Vector2(-1.5f, 0.5f));    
    thrusterParticleVertices.emplace_back(Vector2(-0.6f, -0.6f));  
    thrusterParticleVertices.emplace_back(Vector2(0.0f, -2.0f));    
    thrusterParticleVertices.emplace_back(Vector2(0.6f, -0.6f));    
    thrusterParticleVertices.emplace_back(Vector2(1.5f, 0.5f));    
    thrusterParticleVertices.emplace_back(Vector2(0.6f, 0.6f));     
    
    mThrusterParticles = new ParticleSystemComponent(this, thrusterParticleVertices, 10);
}

void Ship::OnProcessInput(const Uint8* state)
{
    mIsThrusting = false;
    

    if (state[SDL_SCANCODE_W])
    {
        Vector2 forward = GetForward();
        mRigidBodyComponent->ApplyForce(forward * mForwardSpeed);
        mIsThrusting = true;
        
     
        mThrusterFlashTimer += 0.016f;
        mThrusterComponent->SetVisible(Math::Sin(mThrusterFlashTimer * 15.0f) > 0.0f);
        
       
        Vector2 leftEngineOffset = forward * (-mHeight / 2.0f) + Vector2(-mHeight / 12.0f, 0.0f);
        Vector2 rightEngineOffset = forward * (-mHeight / 2.0f) + Vector2(mHeight / 12.0f, 0.0f);
      
        mThrusterParticles->EmitParticle(0.8f, 250.0f, leftEngineOffset);
        mThrusterParticles->EmitParticle(0.8f, 250.0f, rightEngineOffset);
    }
    else
    {
        mThrusterComponent->SetVisible(false);
    }
    
    
    if (state[SDL_SCANCODE_A])
    {
        mRigidBodyComponent->SetAngularSpeed(-mRotationForce);
    }

    else if (state[SDL_SCANCODE_D])
    {
        mRigidBodyComponent->SetAngularSpeed(mRotationForce);
    }
    else
    {
        
        mRigidBodyComponent->SetAngularSpeed(0.0f);
    }
    
    
    if (state[SDL_SCANCODE_SPACE])
    {
        if (mLaserCooldown <= 0.0f)
        {
            Vector2 shipPos = GetPosition();
            float shipRadius = mHeight / 2.0f + 10.0f;
            
            
            Vector2 forward = GetForward();
            Vector2 frontOffset = forward * shipRadius;
            mWeapon->EmitParticle(4.0f, 1300.0f, frontOffset);
            
           
            Vector2 left = Vector2(-forward.y, forward.x); 
            Vector2 leftOffset = left * (shipRadius * 0.8f);
            mWeapon->EmitParticle(3.5f, 1100.0f, leftOffset);
            
            
            Vector2 right = Vector2(forward.y, -forward.x); 
            Vector2 rightOffset = right * (shipRadius * 0.8f);
            mWeapon->EmitParticle(3.5f, 1100.0f, rightOffset);
            
            
            Vector2 backward = forward * -1.0f;
            Vector2 rearOffset = backward * (shipRadius * 0.7f);
            mWeapon->EmitParticle(3.0f, 1000.0f, rearOffset);
            
           
            Vector2 frontLeft = Vector2::Normalize(forward + left) * (shipRadius * 0.9f);
            Vector2 frontRight = Vector2::Normalize(forward + right) * (shipRadius * 0.9f);
            Vector2 rearLeft = Vector2::Normalize(backward + left) * (shipRadius * 0.6f);
            Vector2 rearRight = Vector2::Normalize(backward + right) * (shipRadius * 0.6f);
            
            mWeapon->EmitParticle(3.2f, 1150.0f, frontLeft);
            mWeapon->EmitParticle(3.2f, 1150.0f, frontRight);
            mWeapon->EmitParticle(2.8f, 950.0f, rearLeft);
            mWeapon->EmitParticle(2.8f, 950.0f, rearRight);
            
            
            mLaserCooldown = 0.08f;
        }
    }
}

void Ship::OnUpdate(float deltaTime)
{
    // Update laser cooldown
    if (mLaserCooldown > 0.0f)
    {
        mLaserCooldown -= deltaTime;
        if (mLaserCooldown < 0.0f)
        {
            mLaserCooldown = 0.0f;
        }
    }
    
    // Check collision with asteroids
    for (auto asteroid : GetGame()->GetAsteroids())
    {
        if (mCircleColliderComponent->Intersect(*asteroid->GetComponent<CircleColliderComponent>()))
        {
            // Ship hit asteroid - game over
            GetGame()->Quit();
            break;
        }
    }
}
