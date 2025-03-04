#include "Shaders.h"
#include "UnlitSpriteMaterialShader.h"
#include "SerializationTypes.h"
#include "DebugShader.h"
#include "DebugRectObjectShader.h"

#include "Graphics.h"

#include <cstdint>
#include <vector>



namespace DCore
{

ReturnError Shaders::TryCompileShaders()
{
	{
		uint32_t unlitSpriteMaterialShaderProgram;
		ReturnError error(TryMakeShaderProgram(UnlitSpriteMaterialShader::Get().GetVertexSrc(), nullptr, UnlitSpriteMaterialShader::Get().GetFragmentSrc(), unlitSpriteMaterialShaderProgram));
		if (!error.Ok)
		{
			return error;
		}
		UnlitSpriteMaterialShader::Get().SetProgram(unlitSpriteMaterialShaderProgram);
	}
	{
		uint32_t program;
		ReturnError error(TryMakeShaderProgram(DebugRectObjectShader::Get().GetVertexSrc(), DebugRectObjectShader::Get().GetGeometrySrc(), DebugRectObjectShader::Get().GetFragmentSrc(), program));
		if (!error.Ok)
		{
			return error;
		}
		DebugRectObjectShader::Get().SetProgram(program);
	}
	return ReturnError();
}

ReturnError Shaders::TryMakeShaderProgram(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, uint32_t& outShaderProgram)
{
	ReturnError toReturn;
	GLuint vertexShader(glCreateShader(GL_VERTEX_SHADER));
	glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShader);
	GLint isCompiled(0);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		toReturn.Ok = false;
		glGetShaderInfoLog(vertexShader, STRING_SIZE, nullptr, toReturn.Message.Data());
		glDeleteShader(vertexShader);
		return toReturn;
	}
	GLuint geometryShader(glCreateShader(GL_GEOMETRY_SHADER));
	if (geometrySrc != nullptr)
	{
		glShaderSource(geometryShader, 1, &geometrySrc, nullptr);
		glCompileShader(geometryShader);
		isCompiled = 0;
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			toReturn.Ok = false;
			glGetShaderInfoLog(vertexShader, STRING_SIZE, nullptr, toReturn.Message.Data());
			glDeleteShader(vertexShader);
			glDeleteShader(geometryShader);
			return toReturn;
		}
	}
	GLuint fragmentShader(glCreateShader(GL_FRAGMENT_SHADER));
	glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShader);
	isCompiled = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		toReturn.Ok = false;
		glGetShaderInfoLog(vertexShader, STRING_SIZE, nullptr, toReturn.Message.Data());
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteShader(geometryShader);
		return toReturn;
	}
	GLuint program(glCreateProgram());
	glAttachShader(program, vertexShader);
	if (geometrySrc != nullptr)
	{
		glAttachShader(program, geometryShader);
	}
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	GLint isLinked(0);
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		toReturn.Ok = false;
		glGetProgramInfoLog(program, STRING_SIZE, nullptr, toReturn.Message.Data());
		glDeleteProgram(program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return toReturn;
	}
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	outShaderProgram = program;
	return toReturn;
}

}
