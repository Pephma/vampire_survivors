
#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"

Renderer::Renderer(struct SDL_Window *window)
: mBaseShader(nullptr)
, mWindow(window)
, mContext(nullptr)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float width, float height)
{
    // Specify version 3.3 (core profile)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Enable double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Force OpenGL to use hardware acceleration
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Turn on vsync
    SDL_GL_SetSwapInterval(1);

    // Create an OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }

    // Make sure we can create/compile shaders
    if (!LoadShaders()) {
        SDL_Log("Failed to load shaders.");
        return false;
    }

    // Set the clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Enable alpha blending for textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create orthographic projection matrix
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    mBaseShader->SetActive();
    
    // Set default uniform values
    mBaseShader->SetFloatUniform("uTextureFactor", 0.0f);
    mBaseShader->SetVectorUniform("uTexRect", Vector4(0.0f, 0.0f, 1.0f, 1.0f));

    // Create sprite vertex array for texture rendering
    CreateSpriteVerts();

    return true;
}

void Renderer::Shutdown()
{
    mBaseShader->Unload();
    delete mBaseShader;

    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
}

void Renderer::Clear()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(const class Matrix4 &modelMatrix, VertexArray* vertices, class Vector3 color)
{
    mBaseShader->SetActive();
    mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
    mBaseShader->SetVectorUniform("uColor", color);
    mBaseShader->SetFloatUniform("uTextureFactor", 0.0f);

    vertices->SetActive();
    glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
}

void Renderer::Draw(RendererMode mode, const Matrix4 &modelMatrix, const Vector2 &cameraPos, VertexArray *vertices,
                    const Vector3 &color, Texture *texture, const Vector4 &textureRect, float textureFactor) {
    mBaseShader->SetActive();
    mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
    mBaseShader->SetVectorUniform("uColor", color);
    mBaseShader->SetVectorUniform("uTexRect", textureRect);

    if (texture) {
        // Use sprite vertex array with texture coordinates for textured rendering
        glBindVertexArray(mSpriteVertexArray);
        texture->SetActive();
        mBaseShader->SetFloatUniform("uTextureFactor", textureFactor);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    } else {
        // Use provided vertices for non-textured rendering
        mBaseShader->SetFloatUniform("uTextureFactor", 0.0f);
        if (vertices) {
            vertices->SetActive();
            if (mode == RendererMode::LINES) {
                glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
            } else if (mode == RendererMode::TRIANGLES) {
                glDrawElements(GL_TRIANGLES, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
            }
        }
    }
}

void Renderer::DrawTexture(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                           Texture *texture, const Vector4 &textureRect, const Vector2 &cameraPos, bool flip,
                           float textureFactor) {
    float flipFactor = flip ? -1.0f : 1.0f;

    // Apply camera offset to position
    Vector2 screenCenter(1024.0f / 2.0f, 768.0f / 2.0f); // WINDOW_WIDTH and WINDOW_HEIGHT
    Vector2 cameraOffset = screenCenter - cameraPos;
    Vector2 finalPos = position + cameraOffset;

    Matrix4 model = Matrix4::CreateScale(Vector3(size.x * flipFactor, size.y, 1.0f)) *
                    Matrix4::CreateRotationZ(rotation) *
                    Matrix4::CreateTranslation(Vector3(finalPos.x, finalPos.y, 0.0f));

    Draw(RendererMode::TRIANGLES, model, cameraPos, mSpriteVerts, color, texture, textureRect, textureFactor);
}

void Renderer::DrawFilled(const class Matrix4 &modelMatrix, VertexArray* vertices, class Vector3 color)
{
    mBaseShader->SetActive();
    mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
    mBaseShader->SetVectorUniform("uColor", color);
    mBaseShader->SetFloatUniform("uTextureFactor", 0.0f);

    vertices->SetActive();
    glDrawElements(GL_TRIANGLES, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
}

void Renderer::Present()
{
    // Swap the buffers
    SDL_GL_SwapWindow(mWindow);
}

Texture *Renderer::GetTexture(const std::string &fileName) {
    Texture *tex = nullptr;
    auto iter = mTextures.find(fileName);
    if (iter != mTextures.end()) {
        tex = iter->second;
    } else {
        tex = new Texture();
        if (tex->Load(fileName)) {
            mTextures.emplace(fileName, tex);
            return tex;
        } else {
            delete tex;
            return nullptr;
        }
    }
    return tex;
}


bool Renderer::LoadShaders()
{
    // Create sprite shader
    mBaseShader = new Shader();
    
    // Try different paths for shaders
    std::string shaderPath = "Shaders/Base";
    if (!mBaseShader->Load(shaderPath))
    {
        // Try from parent directory
        shaderPath = "../Shaders/Base";
        if (!mBaseShader->Load(shaderPath))
        {
            // Try from build directory
            shaderPath = "../../Shaders/Base";
            if (!mBaseShader->Load(shaderPath))
            {
                SDL_Log("Failed to load shaders from all paths.");
                return false;
            }
        }
    }

    mBaseShader->SetActive();

    return true;
}

void Renderer::CreateSpriteVerts()
{
    // Create a simple quad for sprite rendering
    // Format: x, y, z, u, v
    float vertices[] = {
        -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, // top left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top right
         0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f  // bottom left
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create vertex array
    glGenVertexArrays(1, &mSpriteVertexArray);
    glBindVertexArray(mSpriteVertexArray);

    // Create and bind vertex buffer
    unsigned int vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and bind index buffer
    unsigned int indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Texture coordinate attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Create a simple VertexArray wrapper (we'll use the raw GL objects above)
    mSpriteVerts = new VertexArray(vertices, 4, indices, 6);
}
