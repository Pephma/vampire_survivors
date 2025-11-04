
#pragma once
#include "Actor.h"

class Asteroid : public Actor
{
public:
    Asteroid(Game* game, float radius, int numVertices = 10, float forwardForce = 1000.0f, int generation = 0);
    ~Asteroid();
    
    void OnDestroy(); 
    int GetGeneration() const { return mGeneration; }

private:
    static std::vector<Vector2> GenerateVertices(int numVertices, float radius);
    static Vector2 GenerateRandomStartingForce(float min, float max);

    static float CalculateAverageVerticesLength(std::vector<Vector2>& vertices);
    
    void CreateExplosionEffect();
    void SplitIntoSmaller();

    int mGeneration; 
    float mRadius;
    
    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    class ParticleSystemComponent* mExplosion;
};
