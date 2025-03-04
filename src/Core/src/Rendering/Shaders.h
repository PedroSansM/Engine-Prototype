#pragma once

#include "ReturnError.h"

#include <cstdint>



namespace DCore
{

class Shaders
{
public:
	~Shaders() = default;
public:
	ReturnError TryCompileShaders();
public:
	static Shaders& Get()
	{
		static Shaders shaders;
		return shaders;
	}
private:
	Shaders() = default;
private:
	ReturnError TryMakeShaderProgram(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, uint32_t& outShaderProgram);
};

}
