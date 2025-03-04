#pragma once

#include "VertexStructures.h"
#include "RendererTypes.h"
#include "Asset.h"
#include "DebugRectObjectShader.h"
#include "Graphics.h"


#include <array>
#include <cstddef>
#include <cstdint>



namespace DCore
{

template <size_t MaxNumberOfObjects> // Its also the maximum batch size.
class DebugRectObjectRenderer
{
public:
	static constexpr size_t maxNumberOfObjects{MaxNumberOfObjects};
	static constexpr size_t numberOfVerticesPerObject{1};
	static constexpr size_t maxNumberOfVertices{numberOfVerticesPerObject * maxNumberOfObjects};
public:
	using vertexBufferType = std::array<DebugRectVertex, maxNumberOfVertices>;
	using objectType = std::array<DebugRectVertex, numberOfVerticesPerObject>;
public:
	DebugRectObjectRenderer()
		:
		m_vertexBufferSize(0),
		m_numberOfObjectsToDraw(0)
	{}
	DebugRectObjectRenderer(const DebugRectObjectRenderer&) = delete;
	DebugRectObjectRenderer(DebugRectObjectRenderer&&) = delete;
	~DebugRectObjectRenderer() = default;
public:
	void Setup()
	{
		glGenBuffers(1, &m_vertexBufferObject);
		glGenVertexArrays(1, &m_vertexArrayObject);
		glBindVertexArray(m_vertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, maxNumberOfVertices * sizeof(DebugRectVertex), nullptr, GL_DYNAMIC_DRAW); CHECK_GL_ERROR;
		constexpr size_t vertexSize{sizeof(DebugRectVertex)};
		size_t offset(0);
		size_t attributeIndex(0);
		// MVP
		for (; attributeIndex < 4; attributeIndex++)
		{
			glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
			glVertexAttribPointer(attributeIndex, 4, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
			offset += 4 * sizeof(float);
		}
		// Offset
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 2, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(DebugRectVertex::Offset);
		// Rect Sizes
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 2, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(DebugRectVertex::RectSizes);
		// Color
		glEnableVertexAttribArray(attributeIndex); CHECK_GL_ERROR;
		glVertexAttribPointer(attributeIndex++, 4, GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const void*>(offset)); CHECK_GL_ERROR;
		offset += sizeof(DebugRectVertex::Color);
	}

	void Submit(const objectType& object)
	{
		m_numberOfObjectsToDraw++;
		DASSERT_E(m_numberOfObjectsToDraw <= maxNumberOfObjects);
		std::memcpy(&m_vertexBuffer[m_vertexBufferSize], object.data(), numberOfVerticesPerObject * sizeof(DebugRectVertex));
		m_vertexBufferSize += numberOfVerticesPerObject;
	}
	
	void Render()
	{
		if (m_numberOfObjectsToDraw == 0)
		{
			return;
		}
		glBindVertexArray(m_vertexArrayObject); CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject); CHECK_GL_ERROR;
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_numberOfObjectsToDraw * numberOfVerticesPerObject * sizeof(DebugRectVertex), static_cast<const void*>(m_vertexBuffer.data())); CHECK_GL_ERROR;
		glUseProgram(DebugRectObjectShader::Get().GetProgram()); CHECK_GL_ERROR;
		glDrawArrays(GL_POINTS, 0, m_numberOfObjectsToDraw * numberOfVerticesPerObject); CHECK_GL_ERROR;
	}

	void Flush()
	{
		m_vertexBufferSize = 0;
		m_numberOfObjectsToDraw = 0;
	}
private:
	vertexBufferType m_vertexBuffer;
	size_t m_vertexBufferSize;
	GLuint m_vertexBufferObject;
	GLuint m_vertexArrayObject;
	size_t m_numberOfObjectsToDraw;
};

}
