#include <SDL.h>
#include "Shader.h"
#include <fstream>
#include <sstream>

Shader::Shader()
: mVertexShader(0)
, mFragShader(0)
, mShaderProgram(0)
{
}

Shader::~Shader()
{
}

bool Shader::Load(const std::string& name)
{
	
	if (!CompileShader(name + ".vert", GL_VERTEX_SHADER, mVertexShader) ||
		!CompileShader(name + ".frag", GL_FRAGMENT_SHADER, mFragShader))
	{
		return false;
	}


	mShaderProgram = glCreateProgram();
	glAttachShader(mShaderProgram, mVertexShader);
	glAttachShader(mShaderProgram, mFragShader);
	glLinkProgram(mShaderProgram);

	return IsValidProgram();
}

void Shader::Unload()
{

	glDeleteProgram(mShaderProgram);
	glDeleteShader(mVertexShader);
	glDeleteShader(mFragShader);

	mShaderProgram = 0;
	mVertexShader = 0;
	mFragShader = 0;
}

void Shader::SetActive() const
{

	glUseProgram(mShaderProgram);
}

void Shader::SetVectorUniform(const char* name, const Vector3& vector) const
{

	GLint loc = glGetUniformLocation(mShaderProgram, name);

	glUniform3fv(loc, 1, vector.GetAsFloatPtr());
}

void Shader::SetMatrixUniform(const char* name, const Matrix4& matrix) const
{

	GLuint loc = glGetUniformLocation(mShaderProgram, name);

	glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.GetAsFloatPtr());
}

bool Shader::CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader)
{
	// Open file
	std::ifstream shaderFile(fileName);
	if (shaderFile.is_open())
	{
		// Read all of the text into a string
		std::stringstream sstream;
		sstream << shaderFile.rdbuf();
		std::string contents = sstream.str();
		const char* contentsChar = contents.c_str();

		// Create a shader of the specified type
		outShader = glCreateShader(shaderType);

        // Set the source characters and try to compile
		glShaderSource(outShader, 1, &(contentsChar), nullptr);
		glCompileShader(outShader);

		if (!IsCompiled(outShader))
		{
			SDL_Log("Failed to compile shader %s", fileName.c_str());
			return false;
		}
	}
	else
	{
		SDL_Log("Shader file not found: %s", fileName.c_str());
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint shader)
{
	GLint status = 0;

    // Query the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		SDL_Log("GLSL Compile Failed:\n%s", buffer);
		return false;
	}

	return true;
}

bool Shader::IsValidProgram() const
{

	GLint status = 0;
	// Query the link status
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
		SDL_Log("GLSL Link Status:\n%s", buffer);
		return false;
	}

	return true;
}
