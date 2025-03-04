#pragma once

#include <cstdint>



namespace DCore
{

class DebugRectObjectShader
{
public:
	DebugRectObjectShader(const DebugRectObjectShader&) = delete;	
	DebugRectObjectShader(DebugRectObjectShader&&) = delete;
	~DebugRectObjectShader();
public:
	static DebugRectObjectShader& Get()
	{
		static DebugRectObjectShader debugRectObjectShader;
		return debugRectObjectShader;
	}
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

	const char* GetGeometrySrc() const
	{
		return m_geometrySrc;
	}

	const char* GetFragmentSrc() const
	{
		return m_fragmentSrc;
	}
private:
	DebugRectObjectShader();
private:
	uint32_t m_program;
	const char* m_vertexSrc;
	const char* m_geometrySrc;
	const char* m_fragmentSrc;
};

}
