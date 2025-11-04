
#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"

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

    // Create orthographic projection matrix
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    mBaseShader->SetActive();

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

    vertices->SetActive();
    glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
}

void Renderer::DrawFilled(const class Matrix4 &modelMatrix, VertexArray* vertices, class Vector3 color)
{
    mBaseShader->SetActive();
    mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
    mBaseShader->SetVectorUniform("uColor", color);

    vertices->SetActive();
    glDrawElements(GL_TRIANGLES, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
}

void Renderer::Present()
{
    // Swap the buffers
    SDL_GL_SwapWindow(mWindow);
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
