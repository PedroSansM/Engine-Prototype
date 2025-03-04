#include "DebugShader.h"
#include "Graphics.h"



namespace DCore
{

DebugShader::DebugShader()
	:
	m_program(0),
	m_vertexSrc(nullptr),
	m_fragmentSrc(nullptr)
{
	m_vertexSrc = R"(
	
		#version 440 core

		layout (location = 0) in uint a_drawOrder;
		layout (location = 1) in vec3 a_pos;
		layout (location = 2) in uint a_end;


		void main()
		{
			gl_Position = vec4(a_pos + a_drawOrder, 1.0);
		}

	)";
	m_fragmentSrc = R"(

		#version 440 core

		out vec4 color;



		void main()
		{
			color = vec4(1.0, 0.0, 1.0, 1.0);
		}

	)";
}

DebugShader::~DebugShader()
{
	glDeleteProgram(m_program);
}

int DebugShader::GetTexturesUniformLocation() const
{
	return glGetUniformLocation(m_program, "u_textures");
}

}
