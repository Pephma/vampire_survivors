
#pragma once
#include "Component.h"
#include "../Math.h"

class RigidBodyComponent : public Component
{
public:
    static const int MAX_VELOCITY = 700; // Normal speed

    // Lower update order to update first
    RigidBodyComponent(class Actor* owner, float mass = 1.0f, int updateOrder = 10);

    void Update(float deltaTime) override;

    // Getters/setters
    const Vector2& GetVelocity() const { return mVelocity; }
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }

    const Vector2& GetAcceleration() const { return mAcceleration; }
    void SetAcceleration(const Vector2& acceleration) { mAcceleration = acceleration; }

    void SetAngularSpeed(const float speed) { mAngularSpeed = speed; }
    float GetAngularSpeed() const { return mAngularSpeed; }

    void ApplyForce(const Vector2 &force);

private:
    void ScreenWrap(Vector2 &position);

    // Physical properties
    float mMass;
    float mAngularSpeed;

    Vector2 mVelocity;
    Vector2 mAcceleration;
};
