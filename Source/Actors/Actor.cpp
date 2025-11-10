// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Actor.h"
#include "../Game.h"
#include "../Components/Component.h"
#include <algorithm>

Actor::Actor(Game* game)
        : mState(ActorState::Active)
        , mPosition(Vector2::Zero)
        , mScale(Vector2(1.0f, 1.0f))
        , mRotation(0.0f)
        , mGame(game)
        , mLifetime(-1.0f) // <-- INICIALIZAÇÃO AQUI
{

    mGame->AddActor(this);
}

Actor::~Actor()
{
    mGame->RemoveActor(this);

    for (auto component : mComponents)
    {
        delete component;
    }
    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        // --- LÓGICA DE TEMPO DE VIDA ---
        if (mLifetime > 0.0f)
        {
            mLifetime -= deltaTime;
            if (mLifetime <= 0.0f)
            {
                SetState(ActorState::Destroy);
                return; // Não precisa atualizar componentes se acabou de morrer
            }
        }
        // -------------------------------

        for (auto component : mComponents)
        {
            component->Update(deltaTime);
        }

        OnUpdate(deltaTime);
    }
}

void Actor::OnUpdate(float deltaTime)
{

}

void Actor::ProcessInput(const Uint8* keyState)
{
   if (mState == ActorState::Active)
    {
        for (auto component : mComponents)
        {
            component->ProcessInput(keyState);
        }

        OnProcessInput(keyState);
    }
}

void Actor::OnProcessInput(const Uint8* keyState)
{

}

void Actor::AddComponent(Component* c)
{
    mComponents.emplace_back(c);

    std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
        return a->GetUpdateOrder() < b->GetUpdateOrder();
    });
}

Matrix4 Actor::GetModelMatrix() const
{
    Matrix4 scaleMat = Matrix4::CreateScale(mScale.x, mScale.y, 1.0f);
    Matrix4 rotMat   = Matrix4::CreateRotationZ(mRotation);
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mPosition.x, mPosition.y, 0.0f));
    return scaleMat * rotMat * transMat;
}