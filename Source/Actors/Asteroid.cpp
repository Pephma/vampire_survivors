#define DB_PERLIN_IMPL

#include "Asteroid.h"
#include "../Game.h"
#include "../Random.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/ParticleSystemComponent.h"

Asteroid::Asteroid(Game* game, const float radius, const int numVertices, const float forwardForce, const int generation)
        :Actor(game)
        ,mRigidBodyComponent(nullptr)
        ,mDrawComponent(nullptr)
        ,mGeneration(generation)
        ,mRadius(radius)
{

    std::vector<Vector2> vertices = GenerateVertices(numVertices, radius);

    float averageRadius = CalculateAverageVerticesLength(vertices);

    mDrawComponent = new DrawComponent(this, vertices);
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, averageRadius);

    Vector2 startingForce = GenerateRandomStartingForce(forwardForce * 0.5f, forwardForce);
    mRigidBodyComponent->ApplyForce(startingForce);

    float angularSpeed = Random::GetFloatRange(-2.0f, 2.0f);
    mRigidBodyComponent->SetAngularSpeed(angularSpeed);

    std::vector<Vector2> explosionVertices;
    explosionVertices.emplace_back(Vector2(-1.0f, -1.0f));      
    explosionVertices.emplace_back(Vector2(1.0f, -1.0f));       
    explosionVertices.emplace_back(Vector2(1.0f, 1.0f));        
    explosionVertices.emplace_back(Vector2(-1.0f, 1.0f));
    
    mExplosion = new ParticleSystemComponent(this, explosionVertices, 8);

    game->AddAsteroid(this);
}

Asteroid::~Asteroid()
{
    GetGame()->RemoveAsteroid(this);
}


std::vector<Vector2> Asteroid::GenerateVertices(int numVertices, float radius)
{
    std::vector<Vector2> vertices;

    for (int i = 0; i < numVertices; i++)
    {
        float baseAngle = (2.0f * Math::Pi * i) / numVertices;

        float angleVariation = Random::GetFloatRange(-0.2f, 0.2f);
        float angle = baseAngle + angleVariation;

        float minRadius = radius * 0.6f; 
        float maxRadius = radius * 1.3f; 
        
        float randomLength = Random::GetFloatRange(minRadius, maxRadius);
 
        float x = randomLength * Math::Cos(angle);
        float y = randomLength * Math::Sin(angle);
        
        vertices.emplace_back(Vector2(x, y));
    }
    
    return vertices;
}

float Asteroid::CalculateAverageVerticesLength(std::vector<Vector2>& vertices)
{
    if (vertices.empty())
    {
        return 0.0f;
    }
    
    float totalLength = 0.0f;
    for (const auto& vertex : vertices)
    {
        totalLength += vertex.Length();
    }
    
    return totalLength / static_cast<float>(vertices.size());
}

Vector2 Asteroid::GenerateRandomStartingForce(float minForce, float maxForce)
{

    float angle = Random::GetFloatRange(0.0f, 2.0f * Math::Pi);

    float forceMagnitude = Random::GetFloatRange(minForce, maxForce);

    float forceX = forceMagnitude * Math::Cos(angle);
    float forceY = forceMagnitude * Math::Sin(angle);
    
    return Vector2(forceX, forceY);
}

void Asteroid::OnDestroy()
{

    CreateExplosionEffect();

    if (mGeneration == 0)
    {
        SplitIntoSmaller();
    }
}

void Asteroid::CreateExplosionEffect()
{

    int numParticles = 3 + (3 - mGeneration) * 2; 
    
    for (int i = 0; i < numParticles; i++)
    {

        float angle = Random::GetFloatRange(0.0f, 2.0f * Math::Pi);
        Vector2 offset = Vector2(Math::Cos(angle), Math::Sin(angle)) * Random::GetFloatRange(2.0f, 5.0f);
        
        float speed = Random::GetFloatRange(50.0f, 150.0f);
        float lifetime = Random::GetFloatRange(0.3f, 0.8f);
        
        mExplosion->EmitParticle(lifetime, speed, offset);
    }
}

void Asteroid::SplitIntoSmaller()
{

    if (mGeneration == 0)
    {
        float newRadius = mRadius * 0.7f;
        int newGeneration = mGeneration + 1;
        
        for (int i = 0; i < 2; i++) 
        {

            float angle = (Math::Pi * i) + Random::GetFloatRange(-0.5f, 0.5f); 
            Vector2 offset = Vector2(Math::Cos(angle), Math::Sin(angle)) * Random::GetFloatRange(20.0f, 30.0f);
            Vector2 newPos = GetPosition() + offset;

            float force = Random::GetFloatRange(150.0f, 250.0f);
            int numVertices = Random::GetIntRange(5, 8); 
            
            Asteroid* newAsteroid = new Asteroid(GetGame(), newRadius, numVertices, force, newGeneration);
            newAsteroid->SetPosition(newPos);

            Vector2 extraForce = GenerateRandomStartingForce(80.0f, 150.0f);
            newAsteroid->GetComponent<RigidBodyComponent>()->ApplyForce(extraForce);
        }
    }

}