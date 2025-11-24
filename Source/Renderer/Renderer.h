#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include "../Math.h"
#include "VertexArray.h"
#include "Texture.h"


enum class RendererMode {
    TRIANGLES,
    LINES
};

class Renderer
{
public:
	Renderer(SDL_Window* window);
	~Renderer();

	bool Initialize(float width, float height);
	void Shutdown();

	void Clear();
	void Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);
	void DrawFilled(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);
    void DrawRect(const Vector2 &position, const Vector2 &size, float rotation,
              const Vector3 &color, const Vector2 &cameraPos, RendererMode mode);
	void DrawWithCamera(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, const Vector2& cameraOffset);
    void DrawTexture(const Vector2 &position, const Vector2 &size, float rotation,
                 const Vector3 &color, Texture *texture,
                 const Vector4 &textureRect = Vector4::UnitRect,
                 const Vector2 &cameraPos = Vector2::Zero, bool flip = false,
                 float textureFactor = 1.0f);

    void DrawGeometry(const Vector2 &position, const Vector2 &size, float rotation,
                      const Vector3 &color, const Vector2 &cameraPos, VertexArray *vertexArray, RendererMode mode);

	void Present();

	// Getters
	class Shader* GetBaseShader() const { return mBaseShader; }
    class Texture *GetTexture(const std::string &fileName);

private:
    void Draw(RendererMode mode, const Matrix4 &modelMatrix, const Vector2 &cameraPos, VertexArray *vertices,
          const Vector3 &color, Texture *texture = nullptr, const Vector4 &textureRect = Vector4::UnitRect,
          float textureFactor = 1.0f);

	bool LoadShaders();

    void CreateSpriteVerts();

	// Game
	class Game* mGame;

	// Sprite shader
	class Shader* mBaseShader;

    // Sprite vertex array
    class VertexArray *mSpriteVerts;
    unsigned int mSpriteVertexArray;

	// Window
	SDL_Window* mWindow;

	// OpenGL context
	SDL_GLContext mContext;

	// Ortho projection for 2D shaders
	Matrix4 mOrthoProjection;

    // Map of textures loaded
    std::unordered_map<std::string, class Texture *> mTextures;
};