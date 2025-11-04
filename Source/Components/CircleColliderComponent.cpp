

#include "CircleColliderComponent.h"
#include "../Actors/Actor.h"
#include "../Math.h"
#include "../Renderer/VertexArray.h"
#include <vector>

CircleColliderComponent::CircleColliderComponent(class Actor* owner, const float radius, const int updateOrder)
        :Component(owner, updateOrder)
        ,mRadius(radius)
{
   
    const int numPoints = 10;
    std::vector<float> vertexArray;
    
    for (int i = 0; i < numPoints; i++)
    {
        float angle = (2.0f * Math::Pi * i) / numPoints;
        float x = mRadius * Math::Cos(angle);
        float y = mRadius * Math::Sin(angle);
        
        vertexArray.push_back(x);
        vertexArray.push_back(y);
        vertexArray.push_back(0.0f); // z coordinate
    }
    
   
    std::vector<unsigned int> indices;
    for (int i = 0; i < numPoints; i++)
    {
        indices.push_back(i);
    }
    
    
    mDrawArray = new VertexArray(vertexArray.data(), numPoints,
                                indices.data(), static_cast<unsigned int>(indices.size()));
}

CircleColliderComponent::~CircleColliderComponent()
{
   
    delete mDrawArray;
    mDrawArray = nullptr;
}

bool CircleColliderComponent::Intersect(const CircleColliderComponent& c) const
{
   
    Vector2 diff = mOwner->GetPosition() - c.GetOwner()->GetPosition();
    float distanceSquared = diff.LengthSq();

    float radiusSum = mRadius + c.GetRadius();
    
    
    return distanceSquared <= (radiusSum * radiusSum);
}

void CircleColliderComponent::DebugDraw(Renderer *renderer)
{
    // Draw the circle collider in green for debugging
    renderer->Draw(mOwner->GetModelMatrix(), mDrawArray, Vector3(0.0f, 1.0f, 0.0f));
}