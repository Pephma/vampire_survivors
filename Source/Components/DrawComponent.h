
#pragma once
#include "Component.h"
#include "../Math.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include <vector>
#include <SDL.h>

class DrawComponent : public Component
{
public:
    
    DrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder = 100);
    ~DrawComponent();

    virtual void Draw(Renderer* renderer);
    int GetDrawOrder() const { return mDrawOrder; }

    void SetVisible(bool visible) { mIsVisible = visible; }
    void SetColor(const Vector3& color) { mColor = color; }
    const Vector3& GetColor() const { return mColor; }
    void SetUseCamera(bool useCamera) { mUseCamera = useCamera; }
    void SetFilled(bool filled) { mUseFilled = filled; }

protected:
    int mDrawOrder;
    bool mIsVisible;
    bool mUseCamera;
    bool mUseFilled;
    Vector3 mColor;
    class VertexArray *mDrawArray;
};
