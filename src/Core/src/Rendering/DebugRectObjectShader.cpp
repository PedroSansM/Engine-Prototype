#include "DebugRectObjectShader.h"
#include "Graphics.h"



namespace DCore
{

DebugRectObjectShader::DebugRectObjectShader()
	:
	m_program(0),
	m_geometrySrc(nullptr)
{
	m_vertexSrc = R"(
		
		# version 440 core
		
		layout (location = 0) in mat4 a_mvp;
		layout (location = 4) in vec2 a_offset;
		layout (location = 5) in vec2 a_rectSizes;
		layout (location = 6) in vec4 a_color;

		out mat4 v_mvp;
		out vec2 v_offset;
		out vec2 v_rectSizes;
		out vec4 v_color;
		
		void main()
		{
			v_mvp = a_mvp;
			v_offset = a_offset;
			v_rectSizes = a_rectSizes;
			v_color = a_color;
			gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	)";
	m_geometrySrc = R"(
		
		#version 440 core

		layout (points) in;
		layout (line_strip, max_vertices = 5) out;

		in mat4 v_mvp[];
		in vec2 v_offset[];
		in vec2 v_rectSizes[];
		in vec4 v_color[];

		out vec4 v_fColor;

		void main()
		{
			mat4 mvp = v_mvp[0];
			vec2 offset = v_offset[0];
			vec2 rectSizes = v_rectSizes[0];
			vec4 color = v_color[0];
			v_fColor = color;
			// Bottom Left
			gl_Position = mvp * vec4(offset.x - rectSizes.x/2.0f, offset.y - rectSizes.y/2.0f, 0.0f, 1.0f);
			EmitVertex();
			// Bottom Right
			gl_Position = mvp * vec4(offset.x + rectSizes.x/2.0f, offset.y - rectSizes.y/2.0f, 0.0f, 1.0f);
			EmitVertex();		
			// Top Right
			gl_Position = mvp * vec4(offset.x + rectSizes.x/2.0f, offset.y + rectSizes.y/2.0f, 0.0f, 1.0f);
			EmitVertex();		
			// Top Left
			gl_Position = mvp * vec4(offset.x - rectSizes.x/2.0f, offset.y + rectSizes.y/2.0f, 0.0f, 1.0f);
			EmitVertex();		
			// Bottom Left
			gl_Position = mvp * vec4(offset.x - rectSizes.x/2.0f, offset.y - rectSizes.y/2.0f, 0.0f, 1.0f);
			EmitVertex();			
			EndPrimitive();
		}

	)";
	m_fragmentSrc = R"(

		#version 440 core
	
		layout (location = 0) out vec4 o_color;

		in flat vec4 v_fColor;

		void main()
		{
			o_color = v_fColor;
		}
	)";
}

DebugRectObjectShader::~DebugRectObjectShader()
{
	glDeleteProgram(m_program);
}

}
