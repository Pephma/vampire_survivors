#include "FloatingText.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/TextRenderer.h"
#include "../Components/DrawComponent.h"

FloatingText::FloatingText(class Game* game, const Vector2& position, const std::string& text, const Vector3& color, float lifetime, float speed)
    : Actor(game)
    , mLifetime(lifetime)
    , mMaxLifetime(lifetime)
    , mSpeed(speed)
    , mText(text)
    , mColor(color)
    , mDrawComponent(nullptr)
{
    SetPosition(position);
    SetState(ActorState::Active);
}

void FloatingText::OnUpdate(float deltaTime)
{
    mLifetime -= deltaTime;
    
    if (mLifetime <= 0.0f)
    {
        SetState(ActorState::Destroy);
        return;
    }
    
    // Float upward
    Vector2 pos = GetPosition();
    pos.y -= mSpeed * deltaTime;
    SetPosition(pos);
    
    // Fade out
    float alpha = mLifetime / mMaxLifetime;
    Vector3 displayColor = mColor;
    displayColor.x *= alpha;
    displayColor.y *= alpha;
    displayColor.z *= alpha;
    
    // Draw text
    auto* renderer = GetGame()->GetRenderer();
    if (renderer)
    {
        Vector2 screenPos = GetPosition() - GetGame()->GetCameraPosition();
        screenPos.x += Game::WINDOW_WIDTH / 2.0f;
        screenPos.y += Game::WINDOW_HEIGHT / 2.0f;
        
        float scale = 0.8f + (1.0f - alpha) * 0.4f; // Grow slightly as it fades
        TextRenderer::DrawText(renderer, mText, screenPos, scale, displayColor);
    }
}

