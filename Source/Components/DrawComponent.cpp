
#include "DrawComponent.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Renderer/VertexArray.h"

DrawComponent::DrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder)
    :Component(owner)
    ,mDrawOrder(drawOrder)
    ,mIsVisible(true)
    ,mUseCamera(true)
    ,mUseFilled(false)
    ,mColor(1.0f, 1.0f, 1.0f)
{

    mOwner->GetGame()->AddDrawable(this);

    
    std::vector<float> vertexArray;
    for (const auto& vertex : vertices)
    {
        vertexArray.push_back(vertex.x);
        vertexArray.push_back(vertex.y);
        vertexArray.push_back(0.0f); // z coordinate (2D game)
    }

   
    std::vector<unsigned int> indices;
    for (size_t i = 0; i < vertices.size(); i++)
    {
        indices.push_back(static_cast<unsigned int>(i));
    }

    
    mDrawArray = new VertexArray(vertexArray.data(), static_cast<unsigned int>(vertices.size()),
                                 indices.data(), static_cast<unsigned int>(indices.size()));
}

DrawComponent::~DrawComponent()
{
    
    mOwner->GetGame()->RemoveDrawable(this);

    
    delete mDrawArray;
    mDrawArray = nullptr;
}

void DrawComponent::Draw(class Renderer *renderer)
{
    if (mIsVisible && mDrawArray)
    {
        Matrix4 modelMatrix = mOwner->GetModelMatrix();
        
        // Apply camera offset only if enabled and game is playing
        if (mUseCamera && mOwner->GetGame()->GetState() == MenuState::Playing)
        {
            // Get camera offset from game
            Vector2 cameraPos = mOwner->GetGame()->GetCameraPosition();
            Vector2 screenCenter(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f, static_cast<float>(Game::WINDOW_HEIGHT) / 2.0f);
            
            // Calculate offset to center camera on screen
            Vector2 cameraOffset = screenCenter - cameraPos;
            
            // Apply camera offset to model matrix
            Matrix4 cameraTransform = Matrix4::CreateTranslation(Vector3(cameraOffset.x, cameraOffset.y, 0.0f));
            modelMatrix = cameraTransform * modelMatrix;
        }
        
        // Use filled rendering if enabled, otherwise use line rendering
        if (mUseFilled)
        {
            renderer->DrawFilled(modelMatrix, mDrawArray, mColor);
        }
        else
        {
            renderer->Draw(modelMatrix, mDrawArray, mColor);
        }
    }
}

