#include "TextRenderer.h"
#include "../Game.h"
#include <cctype>

void TextRenderer::DrawText(Renderer* renderer, const std::string& text, const Vector2& position, float scale, const Vector3& color)
{
    Vector2 currentPos = position;
    float charWidth = 8.0f * scale;
    
    for (char c : text)
    {
        if (static_cast<unsigned char>(c) == 0xC3)
        {
            continue;
        }
        
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
    
    for (auto& v : vertices)
    {
        v.x += position.x;
        v.y += position.y;
    }
    
    std::vector<float> floatArray;
    std::vector<unsigned int> indices;
    
    float lineWidth = scale * 2.0f;
    
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
        Vector2 perp(-dir.y * lineWidth, dir.x * lineWidth);
        
        unsigned int baseIdx = static_cast<unsigned int>(floatArray.size() / 3);
        
        floatArray.push_back((v1.x - perp.x / 2.0f)); floatArray.push_back((v1.y - perp.y / 2.0f)); floatArray.push_back(0.0f);
        floatArray.push_back((v1.x + perp.x / 2.0f)); floatArray.push_back((v1.y + perp.y / 2.0f)); floatArray.push_back(0.0f);
        floatArray.push_back((v2.x - perp.x / 2.0f)); floatArray.push_back((v2.y - perp.y / 2.0f)); floatArray.push_back(0.0f);
        
        indices.push_back(baseIdx);
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);
        
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
    
    float w = 3.0f * scale;
    float h = 5.0f * scale;
    
    unsigned char uc = static_cast<unsigned char>(c);
    
    if (uc == 0xB4 || uc == 0xF4)
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        
        float hatTop = -h * 0.6f;
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, hatTop));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, hatTop));
        
        return vertices;
    }
    
    if (c == ',')
    {
        vertices.emplace_back(Vector2(w/2.0f, h*1.8f)); vertices.emplace_back(Vector2(w/4.0f, h*2.4f));
        return vertices;
    }
    else if (c == '!')
    {
        vertices.emplace_back(Vector2(w/2.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*1.4f));
        vertices.emplace_back(Vector2(w/2.0f, h*1.7f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
        return vertices;
    }
    else if (c == ':')
    {
        vertices.emplace_back(Vector2(w/2.0f, h*0.5f)); vertices.emplace_back(Vector2(w/2.0f, h*0.7f));
        vertices.emplace_back(Vector2(w/2.0f, h*1.3f)); vertices.emplace_back(Vector2(w/2.0f, h*1.5f));
        return vertices;
    }
    
    c = std::toupper(c);
    if (c == '0')
    {
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
    else if (c == 'H')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'E')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'A')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'L')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'T')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w/2.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'R')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'S')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'P')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'D')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w*0.7f, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w*0.7f, h*2.0f));
        vertices.emplace_back(Vector2(w*0.7f, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w*0.7f, h*2.0f)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'I')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w/2.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'N')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'O')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'U')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'M')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'X')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
    }
    else if (c == 'Y')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w/2.0f, h)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'C')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'F')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
    }
    else if (c == 'G')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'V')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w/2.0f, h*2.0f));
    }
    else if (c == 'W')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w/2.0f, h));
        vertices.emplace_back(Vector2(w, h*2.0f)); vertices.emplace_back(Vector2(w, 0.0f));
    }
    else if (c == 'B')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h));
        vertices.emplace_back(Vector2(w, h)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'K')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, h)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'J')
    {
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'Z')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    else if (c == 'Q')
    {
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(w, 0.0f));
        vertices.emplace_back(Vector2(0.0f, 0.0f)); vertices.emplace_back(Vector2(0.0f, h*2.0f));
        vertices.emplace_back(Vector2(w, 0.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(0.0f, h*2.0f)); vertices.emplace_back(Vector2(w, h*2.0f));
        vertices.emplace_back(Vector2(w*0.7f, h*1.4f)); vertices.emplace_back(Vector2(w, h*2.0f));
    }
    
    return vertices;
}
