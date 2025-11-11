#include "DelayedExplosion.h"
#include "../Game.h"
#include "Player.h"
#include "../Components/DrawComponent.h"
#include "../Math.h"
#include "../Random.h" // Se quiser posições aleatórias

DelayedExplosion::DelayedExplosion(Game* game, Vector2 position, float delay, float radius)
    : Actor(game)
    , mTimer(delay)
    , mRadius(radius)
{
    SetPosition(position);

    // Cria o anel de AVISO
    std::vector<Vector2> vertices;
    const int numVertices = 32;
    for (int i = 0; i < numVertices; ++i)
    {
        float angle = (Math::TwoPi / numVertices) * i;
        vertices.emplace_back(Vector2(Math::Cos(angle) * mRadius, Math::Sin(angle) * mRadius));
    }
    vertices.emplace_back(vertices[0]); // Fecha o loop

    auto* drawComp = new DrawComponent(this, vertices);
    drawComp->SetColor(Vector3(1.0f, 0.1f, 0.1f)); // Vermelho brilhante
    drawComp->SetFilled(false); // Apenas o anel
    drawComp->SetUseCamera(true);
}

void DelayedExplosion::OnUpdate(float deltaTime)
{
    mTimer -= deltaTime;
    if (mTimer <= 0.0f)
    {
        // Tempo acabou! HORA DE EXPLODIR
        auto* player = GetGame()->GetPlayer();
        if (player)
        {
            float dist = (player->GetPosition() - GetPosition()).Length();
            if (dist <= mRadius)
            {
                player->TakeDamage(30.0f); // Dano da bomba
            }
        }

        // Efeitos visuais da explosão
        GetGame()->SpawnExplosionParticles(GetPosition(), Vector3(1.0f, 0.2f, 0.2f));
        GetGame()->AddScreenShake(4.0f, 0.15f);

        // Se destrói
        SetState(ActorState::Destroy);
    }
}