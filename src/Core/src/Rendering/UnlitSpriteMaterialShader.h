#pragma once

#include <cstdint>


namespace DCore
{

class UnlitSpriteMaterialShader
{
public:
	UnlitSpriteMaterialShader(const UnlitSpriteMaterialShader&) = delete;
	UnlitSpriteMaterialShader(const UnlitSpriteMaterialShader&&) = delete;
	~UnlitSpriteMaterialShader();
public:
	static UnlitSpriteMaterialShader& Get()
	{
		static UnlitSpriteMaterialShader unlitSpriteMaterialShader;
		return unlitSpriteMaterialShader;
	}
public:
	int GetTexturesUniformLocation() const;
public:
	uint32_t GetProgram() const
	{
		return m_program;
	}

	void SetProgram(uint32_t program)
	{
		m_program = program;
	}
	
	const char* GetVertexSrc() const
	{
		return m_vertexSrc;
	}

	const char* GetFragmentSrc() const
	{
		return m_fragmentSrc;
	}
private:
	UnlitSpriteMaterialShader();
private:
	uint32_t m_program;
	const char* m_vertexSrc;
	const char* m_fragmentSrc;
};

}
