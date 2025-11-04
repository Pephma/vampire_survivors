#pragma once
#include "../Math.h"
#include "Renderer.h"
#include "VertexArray.h"
#include <string>
#include <vector>

class TextRenderer
{
public:
    static void DrawText(Renderer* renderer, const std::string& text, const Vector2& position, float scale, const Vector3& color);
    static void DrawChar(Renderer* renderer, char c, const Vector2& position, float scale, const Vector3& color);
    
private:
    static std::vector<Vector2> GetCharVertices(char c, float scale);
};

