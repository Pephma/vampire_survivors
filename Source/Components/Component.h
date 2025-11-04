// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <SDL_stdinc.h>

class Component
{
public:
    
    Component(class Actor* owner, int updateOrder = 100);

    virtual ~Component();
    
    virtual void Update(float deltaTime);
    
    virtual void ProcessInput(const Uint8* keyState);
   
    virtual void DebugDraw(class Renderer* renderer);

    int GetUpdateOrder() const { return mUpdateOrder; }
    class Actor* GetOwner() const { return mOwner; }
    class Game* GetGame() const;

protected:
    // Owning actor
    class Actor* mOwner;
    // Update order
    int mUpdateOrder;
};
