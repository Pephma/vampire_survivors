
#pragma once

#include "../Actors/Actor.h"
#include "Component.h"
#include <vector>

class Particle : public Actor
{
public:
    Particle(class Game* game, std::vector<Vector2> &vertices);

    void OnUpdate(float deltaTime) override;

    bool IsDead() const { return mIsDead; }
    void Awake(const Vector2 &position, float rotation, float lifetime = 1.0f);
    void Kill();

private:
    float mLifeTime;
    bool mIsDead;

    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
};

class ParticleSystemComponent : public Component {

public:
    ParticleSystemComponent(class Actor* owner, std::vector<Vector2> &vertices,  int poolSize = 100, int updateOrder = 10);
    void EmitParticle(float lifetime, float speed, const Vector2& offsetPosition = Vector2::Zero);

private:
    std::vector<class Particle*> mParticles;
};
