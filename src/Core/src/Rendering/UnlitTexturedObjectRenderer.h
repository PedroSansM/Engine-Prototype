#pragma once

#include "DCoreAssert.h"
#include "VertexStructures.h"
#include "UnlitSpriteMaterialShader.h"
#include "SparseSet.h"
#include "RendererTypes.h"
#include "TextureUnits.h"
#include "Graphics.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <array>
#include <cstring>



namespace DCore
{

template <size_t MaxNumberOfObjects> // Its also the maximum batch size.
class UnlitTexturedObjectRenderer
{
public:
	static constexpr size_t maxNumberOfObjects{MaxNumberOfObjects};
	static constexpr size_t numberOfVerticesPerObject{4};
	static constexpr size_t numberOfIndicesPerObject{6};
	static constexpr size_t maxNumberOfVertices{numberOfVerticesPerObject * maxNumberOfObjects};
	static constexpr size_t maxNumberOfIndices{numberOfIndicesPerObject * maxNumberOfObjects};
public:
	using vertexBufferType = std::array<UnlitTexturedVertex, maxNumberOfVertices>;
	using indexBufferType = std::array<uint32_t, maxNumberOfIndices>;
	using quadType = std::array<UnlitTexturedVertex, numberOfVerticesPerObject>;
public:
	UnlitTexturedObjectRenderer()
		:
		m_vertexBufferSize(0),
		m_indexBufferSize(0),
		m_beginIndex(0),
		m_endIndex(0)
	{}
	UnlitTexturedObjectRenderer(const UnlitTexturedObjectRenderer&) = delete;
	UnlitTexturedObjectRenderer(UnlitTexturedObjectRenderer&&) = delete;
	~UnlitTexturedObjectRenderer() = default;
public:
	void Setup()
	{
		glGenBuffers(1, &m_vertexBufferObject); CHECK_GL_ERROR;
		glGenBuffers(1, &m_indexBufferObject); CHECK_GL_ERROR;
		glGenVertexArrays(1, &m_vertexArrayObject); CHECK_GL_ERROR;
		glBindVertexArray(m_vertexArrayObject); CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject); CHECK_GL_ERROR;
		glBufferData(GL_ARRAY_BUFFER, maxNumberOfVertices * sizeof(UnlitTexturedVertex), nullptr, GL_DYNAMIC_DRAW); CHECK_GL_ERROR;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject); CHECK_GL_ERROR;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxNumberOfIndices * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW); CHECK_GL_ERROR;
		constexpr size_t vertexSize{sizeof(UnlitTexturedVertex)};
		size_t offset(0);
		size_t attributeIndex(0);
		// Draw Order
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::DrawOrder);
		// MVP
		for (; attributeIndex < 5; attributeIndex++)
		{
			glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
			glVertexAttribPointer(attributeIndex, 4, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
			offset += 4 * sizeof(float);
		}
		// Vertex Pos
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 3, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::VertexPos);
		// Diffuse Color
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 4, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::DiffuseColor);
		// Tint Color
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 4, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::TintColor);
		// To Use Diffuse Tex
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::ToUseDiffuseTex);
		// Diffuse Tex Id
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::DiffuseTexId);	
		// UV
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 2, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::UV);	
		// Entity Id
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::EntityId);
		// Entity Version
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::EntityVersion);
		// Scene Id
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::SceneId);
		// Scene Version
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribIPointer(attributeIndex++, 1, GL_UNSIGNED_INT, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(UnlitTexturedVertex::SceneVersion);
	}

	void Submit(const quadType& vertices)
	{
		m_numberOfObjectsToDraw++;
		DASSERT_E(m_numberOfObjectsToDraw < maxNumberOfObjects);
		uint32_t indices[]{0, 1, 2, 2, 3, 0};
		indices[0] += numberOfVerticesPerObject * (m_numberOfObjectsToDraw - 1);
		indices[1] += numberOfVerticesPerObject * (m_numberOfObjectsToDraw - 1);
		indices[2] += numberOfVerticesPerObject * (m_numberOfObjectsToDraw - 1);
		indices[3] += numberOfVerticesPerObject * (m_numberOfObjectsToDraw - 1);
		indices[4] += numberOfVerticesPerObject * (m_numberOfObjectsToDraw - 1);
		indices[5] += numberOfVerticesPerObject * (m_numberOfObjectsToDraw - 1);
		std::memcpy(&m_vertexBuffer[m_vertexBufferSize], vertices.data(), numberOfVerticesPerObject * sizeof(UnlitTexturedVertex));
		m_vertexBufferSize += numberOfVerticesPerObject;
		std::memcpy(&m_indexBuffer[m_indexBufferSize], indices, numberOfIndicesPerObject * sizeof(uint32_t));
		m_indexBufferSize += numberOfIndicesPerObject;
	}
	
	void Prepare()
	{
		if (m_numberOfObjectsToDraw == 0)
		{
			return;
		}
		bool switched(false);
		do
		{
			switched = false;
			for (size_t i(0); i < m_numberOfObjectsToDraw - 1; i++)
			{
				const size_t leftObjectIndex(i * numberOfVerticesPerObject);
				const size_t rightObjectIndex((i + 1) * numberOfVerticesPerObject);
				if (m_vertexBuffer[leftObjectIndex].DrawOrder > m_vertexBuffer[rightObjectIndex].DrawOrder)
				{
					quadType temp;
					std::memcpy(temp.data(), &m_vertexBuffer[leftObjectIndex], sizeof(quadType));
					std::memcpy(&m_vertexBuffer[leftObjectIndex], &m_vertexBuffer[rightObjectIndex], sizeof(quadType));
					std::memcpy(&m_vertexBuffer[rightObjectIndex], temp.data(), sizeof(quadType));
					switched = true;
				}
			}
		} while(switched);
	}

	void Render()
	{
		const uint32_t currentDrawOrder(m_vertexBuffer[m_beginIndex * numberOfVerticesPerObject].DrawOrder);
		while (true)
		{
			// Advance endIndex until:
			//     The end of the buffers is reached.
			//     A different draw order is found.
			//     There is no more texture slots available.
			bool toBreak(false);
			for (; true; m_endIndex++)
			{
				const size_t vertexBufferEndIndex(m_endIndex * numberOfVerticesPerObject);
				if (m_endIndex >= m_numberOfObjectsToDraw)
				{
					toBreak = true;
					break;
				}
				if (m_vertexBuffer[vertexBufferEndIndex].DrawOrder != currentDrawOrder)
				{
					toBreak = true;
					break;
				}
				if (UnlitTexturedVertex* vertex{&m_vertexBuffer[vertexBufferEndIndex]}; 
					vertex->ToUseDiffuseTex == 1) 
				{
					if (m_textureIds.Size() >= TextureUnits::numberOfTextureUnits)
					{
						break;
					}
					if (!m_textureIds.Exists(vertex->DiffuseTexId))
					{
						m_textureIds.Add(vertex->DiffuseTexId);
						glActiveTexture(GL_TEXTURE0 + m_textureIds.GetIndexTo(vertex->DiffuseTexId)); CHECK_GL_ERROR;
						glBindTexture(GL_TEXTURE_2D, vertex->DiffuseTexId); CHECK_GL_ERROR;
					}
					vertex[0].DiffuseTexId = m_textureIds.GetIndexTo(vertex->DiffuseTexId);
					vertex[1].DiffuseTexId = vertex[0].DiffuseTexId;
					vertex[2].DiffuseTexId = vertex[0].DiffuseTexId;
					vertex[3].DiffuseTexId = vertex[0].DiffuseTexId;
				}
			}
			glBindVertexArray(m_vertexArrayObject); CHECK_GL_ERROR;
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject); CHECK_GL_ERROR;
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_numberOfObjectsToDraw * numberOfVerticesPerObject * sizeof(UnlitTexturedVertex), static_cast<const void*>(m_vertexBuffer.data())); CHECK_GL_ERROR;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObject); CHECK_GL_ERROR;
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (m_endIndex - m_beginIndex) * numberOfIndicesPerObject * sizeof(uint32_t), static_cast<const void*>(m_indexBuffer.data() + m_beginIndex * numberOfIndicesPerObject)); CHECK_GL_ERROR;
			glUseProgram(UnlitSpriteMaterialShader::Get().GetProgram()); CHECK_GL_ERROR;
			if (m_textureIds.Size() > 0)
			{
				glUniform1iv(UnlitSpriteMaterialShader::Get().GetTexturesUniformLocation(), TextureUnits::numberOfTextureUnits, static_cast<const GLint*>(TextureUnits::textureUnits)); CHECK_GL_ERROR;
			}
			glDrawElements(GL_TRIANGLES, (m_endIndex - m_beginIndex) * numberOfIndicesPerObject, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0)); CHECK_GL_ERROR;
			m_beginIndex = m_endIndex;
			m_textureIds.Clear();
			if (toBreak)
			{
				break;
			}
		}
	}

	void Flush()
	{
		m_vertexBufferSize = 0;
		m_indexBufferSize = 0;
		m_numberOfObjectsToDraw = 0;
		m_textureIds.Clear();
		m_beginIndex = 0;
		m_endIndex = 0;
	}
private:
	vertexBufferType m_vertexBuffer;
	indexBufferType m_indexBuffer;
	size_t m_vertexBufferSize;
	size_t m_indexBufferSize;
	GLuint m_vertexBufferObject;
	GLuint m_indexBufferObject;
	GLuint m_vertexArrayObject;
	size_t m_numberOfObjectsToDraw;
	SparseSet<uint32_t> m_textureIds;
	// Determine the range of the elements to draw. [m_beginIndex, m_endIndex) -> excludes endIndex. 
	// Does not map directly into m_vertexBuffer nor m_indexBuffer. Are used like a object index.
	size_t m_beginIndex;
	size_t m_endIndex;
};

}
