#pragma once

#include <cstdint>



namespace DCore
{

class DebugShader
{
public:
	~DebugShader();
public:
	inline static DebugShader& Get()
	{
		static DebugShader debugShader;
		return debugShader;
	}
public:
	int GetTexturesUniformLocation() const;
public:
	inline uint32_t GetProgram() const
	{
		return m_program;
	}

	inline void SetProgram(uint32_t program)
	{
		m_program = program;
	}

	inline const char* GetVertexSrc() const
	{
		return m_vertexSrc;
	}

	inline const char* GetFragmentSrc() const
	{
		return m_fragmentSrc;
	}
private:
	DebugShader();
private:
	uint32_t m_program;
	const char* m_vertexSrc;
	const char* m_fragmentSrc;
};

}
