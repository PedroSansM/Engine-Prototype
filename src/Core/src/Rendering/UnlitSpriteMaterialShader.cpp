#include "UnlitSpriteMaterialShader.h"
#include "Shaders.h"
#include "Graphics.h"



namespace DCore
{

UnlitSpriteMaterialShader::UnlitSpriteMaterialShader()
{
	m_vertexSrc = R"(
	
		#version 440 core

		layout (location = 0) in uint a_drawOrder;
		layout (location = 1) in mat4 a_mvp;
		layout (location = 5) in vec3 a_vertexPos;
		layout (location = 6) in vec4 a_diffuseColor;
		layout (location = 7) in vec4 a_tintColor;
		layout (location = 8) in uint a_toUseDiffuseTex;
		layout (location = 9) in int a_diffuseTexId;
		layout (location = 10) in vec2 a_uv;
		layout (location = 11) in uint a_entityId;
		layout (location = 12) in uint a_entityVersion;
		layout (location = 13) in uint a_sceneId;
		layout (location = 14) in uint a_sceneVersion;

		out vec4 v_diffuseColor;
		out vec4 v_tintColor;
		out uint v_toUseDiffuseTex;
		out int v_diffuseTexId;
		out vec2 v_uv;
		out uint v_entityId;
		out uint v_entityVersion;
		out uint v_sceneId;
		out uint v_sceneVersion;

		void main()
		{
			v_diffuseColor = a_diffuseColor;
			v_tintColor = a_tintColor;
			v_toUseDiffuseTex = a_toUseDiffuseTex;
			v_diffuseTexId = a_diffuseTexId;
			v_uv = a_uv;
			v_entityId = a_entityId;
			v_entityVersion = a_entityVersion;
			v_sceneId = a_sceneId;
			v_sceneVersion = a_sceneVersion;
			gl_Position = a_mvp * vec4(a_vertexPos, 1.0f);
		}

	)";
	m_fragmentSrc = R"(

		#version 440 core

		layout (location = 0) out vec4 o_color;
		layout (location = 1) out ivec4 o_entityInfo;

		in flat vec4 v_diffuseColor;
		in flat vec4 v_tintColor;
		in flat uint v_toUseDiffuseTex;
		in flat int v_diffuseTexId;
		in vec2 v_uv;
		in flat uint v_entityId;
		in flat uint v_entityVersion;
		in flat uint v_sceneId;
		in flat uint v_sceneVersion;

		uniform sampler2D u_textures[16];

		void main()
		{
			if (v_toUseDiffuseTex == 1)
			{
				o_color = texture(u_textures[v_diffuseTexId], v_uv) * v_tintColor;
			}
			else
			{
				o_color = v_diffuseColor * v_tintColor;
			}
			if (o_color.a < 0.1)
			{
				discard;
			}
			else
			{
				o_entityInfo = ivec4(v_entityId, v_entityVersion, v_sceneId, v_sceneVersion);
			}
		}

	)";
}

UnlitSpriteMaterialShader::~UnlitSpriteMaterialShader()
{
	glDeleteProgram(m_program);
}

int UnlitSpriteMaterialShader::GetTexturesUniformLocation() const
{
	return glGetUniformLocation(m_program, "u_textures");
}

}
