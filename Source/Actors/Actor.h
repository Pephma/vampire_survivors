// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <vector>
#include <SDL_stdinc.h>
#include "../Math.h"
#include "../Renderer/Renderer.h"

enum class ActorState
{
    Active,
    Paused,
    Destroy
};

class Actor
{
public:
    Actor(class Game* game);
    virtual ~Actor();

    void Update(float deltaTime);
    void ProcessInput(const Uint8* keyState);

    const Vector2& GetPosition() const { return mPosition; }
    void SetPosition(const Vector2& pos) { mPosition = pos; }

    const Vector2& GetScale() const { return mScale; }
    void SetScale(const Vector2& scale) { mScale = scale; }

    float GetRotation() const { return mRotation; }
    void SetRotation(float rotation) { mRotation = rotation; }

    ActorState GetState() const { return mState; }
    void SetState(ActorState state) { mState = state; }

    Vector2 GetForward() const { return Vector2(Math::Cos(mRotation), Math::Sin(mRotation)); }

    Matrix4 GetModelMatrix() const;

    class Game* GetGame() { return mGame; }

    const std::vector<class Component*>& GetComponents() const { return mComponents; }

    template <typename T>
    T* GetComponent() const
    {
        for (auto c : mComponents)
        {
            T* t = dynamic_cast<T*>(c);
            if (t != nullptr)
            {
                return t;
            }
        }

        return nullptr;
    }

protected:
    class Game* mGame;

    virtual void OnUpdate(float deltaTime);
    virtual void OnProcessInput(const Uint8* keyState);

    ActorState mState;

    Vector2 mPosition;
    Vector2 mScale;
    float mRotation;

    std::vector<class Component*> mComponents;

private:
    friend class Component;

    void AddComponent(class Component* c);
};