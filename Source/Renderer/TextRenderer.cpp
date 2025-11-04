#include "TextRenderer.h"
#include "../Game.h"
#include <cctype>

void TextRenderer::DrawText(Renderer* renderer, const std::string& text, const Vector2& position, float scale, const Vector3& color)
{
    Vector2 currentPos = position;
    float charWidth = 8.0f * scale;
    
    for (char c : text)
    {
        if (c == ' ')
        {
            currentPos.x += charWidth * 0.6f;
            continue;
        }
        
        DrawChar(renderer, c, currentPos, scale, color);
        currentPos.x += charWidth;
    }
}

void TextRenderer::DrawChar(Renderer* renderer, char c, const Vector2& position, float scale, const Vector3& color)
{
    std::vector<Vector2> vertices = GetCharVertices(c, scale);
    if (vertices.empty())
        return;
    
    // Translate vertices to position
    for (auto& v : vertices)
    {
        v.x += position.x;
        v.y += position.y;
    }
    
    // Convert line segments to thick filled lines using triangles
    std::vector<float> floatArray;
    std::vector<unsigned int> indices;
    
    float lineWidth = scale * 2.0f; // Thicker lines for better visibility
    
    for (size_t i = 0; i < vertices.size(); i += 2)
    {
        if (i + 1 >= vertices.size())
            break;
            
        Vector2 v1 = vertices[i];
        Vector2 v2 = vertices[i + 1];
        Vector2 dir = v2 - v1;
        float length = dir.Length();
        if (length < 0.01f)
            continue;
            
        dir.Normalize();
        Vector2 perp(-dir.y * lineWidth, dir.x * lineWidth); // Perpendicular for thickness
        
        unsigned int baseIdx = static_cast<unsigned int>(floatArray.size() / 3);
        
        // Create a rectangle from the line segment (two triangles)
        // Triangle 1
        floatArray.push_back((v1.x - perp.x / 2.0f)); floatArray.push_back((v1.y - perp.y / 2.0f)); floatArray.push_back(0.0f);
        floatArray.push_back((v1.x + perp.x / 2.0f)); floatArray.push_back((v1.y + perp.y / 2.0f)); floatArray.push_back(0.0f);
        floatArray.push_back((v2.x - perp.x / 2.0f)); floatArray.push_back((v2.y - perp.y / 2.0f)); floatArray.push_back(0.0f);
        
        indices.push_back(baseIdx);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);
        
        // Triangle 2
        floatArray.push_back((v1.x + perp.x / 2.0f)); floatArray.push_back((v1.y + perp.y / 2.0f)); floatArray.push_back(0.0f);
        floatArray.push_back((v2.x + perp.x / 2.0f)); floatArray.push_back((v2.y + perp.y / 2.0f)); floatArray.push_back(0.0f);
        floatArray.push_back((v2.x - perp.x / 2.0f)); floatArray.push_back((v2.y - perp.y / 2.0f)); floatArray.push_back(0.0f);
        
        indices.push_back(baseIdx + 3);
        indices.push_back(baseIdx + 4);
        indices.push_back(baseIdx + 5);
    }
    
    if (floatArray.empty())
        return;
    
    Matrix4 matrix = Matrix4::Identity;
    VertexArray va(floatArray.data(), static_cast<unsigned int>(floatArray.size() / 3), indices.data(), static_cast<unsigned int>(indices.size()));
    renderer->DrawFilled(matrix, &va, color);
}

std::vector<Vector2> TextRenderer::GetCharVertices(char c, float scale)
{
    std::vector<Vector2> vertices;
    c = std::toupper(c);
    
    // Simple 7-segment style font
    // Each character is 8x10 pixels
    float w = 3.0f * scale;
    float h = 5.0f * scale;
    
    if (c == '0')
    {
        // Top, bottom, left top, left bottom, right top, right bottom
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == '1')
    {
        vertices.emplace_back(Vector2(w/2.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == '2')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '3')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '4')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '5')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '6')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '7')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '8')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '9')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == '+')
    {
        vertices.emplace_back(Vector2(w/2.0f, h*0.3f)); vertices.emplace_back(Vector2(w/2.0f, h*1.7f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == '%')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'H' || c == 'h')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'E' || c == 'e')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'A' || c == 'a')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'L' || c == 'l')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'T' || c == 't')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w/2.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'R' || c == 'r')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'S' || c == 's')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'P' || c == 'p')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'D' || c == 'd')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w*0.7f, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w*0.7f, h*2.0f));
        vertices.emplace_back(Vector2(w*0.7f, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w*0.7f, h*2.0f)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'I' || c == 'i')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w/2.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'N' || c == 'n')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'O' || c == 'o')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'U' || c == 'u')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'M' || c == 'm')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'X' || c == 'x')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
    }
    else if (c == 'Y' || c == 'y')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w/2.0f, h)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'C' || c == 'c')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'F' || c == 'f')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'G' || c == 'g')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'V' || c == 'v')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'W' || c == 'w')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w, h*2.0f)); vertices.emplace_back(Vector2(w, 0.0f));
    }
    else if (c == 'B' || c == 'b')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'K' || c == 'k')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'J' || c == 'j')
    {
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'Z' || c == 'z')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'Q' || c == 'q')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w*0.7f, h*1.4f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == ':')
    {
        vertices.emplace_back(Vector2(w/2.0f, h*0.5f)); vertices.emplace_back(Vector2(w/2.0f, h*0.7f));
        vertices.emplace_back(Vector2(w/2.0f, h*1.3f)); vertices.emplace_back(Vector2(w/2.0f, h*1.5f));
    }
    
    return vertices;
}

