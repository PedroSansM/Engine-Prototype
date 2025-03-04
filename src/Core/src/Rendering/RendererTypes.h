#pragma once

#include "SceneTypes.h"
#include "TemplateUtils.h"
#include "DCoreAssert.h"
#include "Graphics.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <array>
#include <functional>



namespace DCore
{

using Texture2DIdType = size_t;
using Texture2DVersionType = size_t;

using SpriteMaterialIdType = uint32_t;
using SpriteMaterialVersionType = uint32_t;

#ifndef NDEBUG
#define CHECK_GL_ERROR																					\
{																										\
	switch (glGetError())																				\
	{																									\
	case GL_INVALID_OPERATION:																			\
		DASSERT_E(false && "Invalid operation.");														\
		break;																							\
	case GL_INVALID_ENUM:																				\
		DASSERT_E(false && "Invalid enum.");															\
		break;																							\
	case GL_INVALID_VALUE:																				\
		DASSERT_E(false && "Invalid value.");															\
		break;																							\
	case GL_OUT_OF_MEMORY:																				\
		DASSERT_E(false && "Out of memory.");															\
		break;																							\
	case GL_INVALID_FRAMEBUFFER_OPERATION:																\
		DASSERT_E(false && "Invalid framebuffer operation");											\
		break;																							\
	}																									\
}
#else
#define CHECK_GL_ERROR
#endif

struct VAttrFloat
{
	using type = float;

	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_FLOAT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrFloat2
{
	using type = glm::vec2;

	static constexpr size_t numberOfComponents{2};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_FLOAT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrFloat3
{
	using type = glm::vec3;

	static constexpr size_t numberOfComponents{3};
	static constexpr size_t size{sizeof(type)};
	static constexpr auto glType{GL_FLOAT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrFloat4
{
	using type = glm::vec4;

	static constexpr size_t numberOfComponents{4};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_FLOAT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrMat4
{	
	using type = glm::mat4;

	static constexpr size_t numberOfComponents{16};
	static constexpr size_t size{sizeof(type)};
	static constexpr auto glType{GL_FLOAT};
	static constexpr size_t numberOfAttributes{4};
};

struct VAttrBool
{
	using type = uint32_t;

	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_UNSIGNED_INT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrUInt
{
	using type = uint32_t;
		
	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_UNSIGNED_INT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrInt
{
	using type = int;
		
	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_UNSIGNED_INT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrEntityId
{
	using type = int;

	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType = GL_INT;
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrEntityVersion
{
	using type = int;

	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_INT};
	static constexpr size_t numberOfAttributes{1};
};

struct VAttrSceneId
{	
	using type = int;

	static constexpr size_t numberOfComponents{1};
	static constexpr size_t size{sizeof(type)};	
	static constexpr auto glType{GL_INT};
	static constexpr size_t numberOfAttributes{1};
};

template <class ...>
struct VertexAttributesTotalSize
{
	static constexpr size_t size = 0;
};

template <class VertexAttribute, class ...VertexAttributes>
struct VertexAttributesTotalSize<VertexAttribute, VertexAttributes...>
{
	static constexpr size_t size = VertexAttribute::size + VertexAttributesTotalSize<VertexAttributes...>::size;
};

template <class ...>
struct VertexNumberOfAttributes
{
	static constexpr size_t value = 0;
};

template <class VertexAttribute, class ...VertexAttributes>
struct VertexNumberOfAttributes<VertexAttribute, VertexAttributes...>
{
	static constexpr size_t value = VertexAttribute::numberOfAttributes + VertexNumberOfAttributes<VertexAttributes...>::value;
};

template <size_t Index, class...>
struct VertexAttributePartialSize
{
	static constexpr size_t size = 0;
};

template <size_t Index, class VertexAttribute, class ...VertexAttributes>
struct VertexAttributePartialSize<Index, VertexAttribute, VertexAttributes...>
{
	static constexpr size_t size = VertexAttribute::size + VertexAttributePartialSize<Index - 1, VertexAttributes...>::size;
};

template <class VertexAttribute, class ...VertexAttributes>
struct VertexAttributePartialSize<0, VertexAttribute, VertexAttributes...>
{
	static constexpr size_t size = 0;
};

//emplate <class VertexAttribute, class ...VertexAttributes>
//truct GeneralQuad
//
//static constexpr size_t vertexSize = VertexAttributesTotalSize<VertexAttribute, VertexAttributes...>::size;

//char BottomLeft[vertexSize];
//char BottomRight[vertexSize];
//char TopRight[vertexSize];
//char TopLeft[vertexSize];
//;

template <class TextureIdsIndexITypeList, class ...VertexAttributes>
struct GeneralQuad
{	
	static_assert(std::is_same<VAttrUInt, typename TypeList<VertexAttributes...>::head>::value, "The first attribute of the vertices of a general quad must be its draw order (type VAttrUInt).");
	static_assert(TypeList<VertexAttributes...>::size != 0, "No attributes passed to general quad.");

	using Vertex = RuntimeTypes<typename VertexAttributes::type...>;
	using AttributesTypeList = TypeList<VertexAttributes...>;
	using TextureIdsIndices = TextureIdsIndexITypeList;
	using DrawOrderComparatorType = GeneralQuad<TextureIdsIndexITypeList, VertexAttributes...>;

	static constexpr size_t vertexSize{offsetof(GeneralQuad, BottomRight)};
	static constexpr size_t numberOfIndices{6};
	static constexpr size_t numberOfVertices{4};
	static constexpr size_t numberOfAttributes{VertexNumberOfAttributes<VertexAttributes...>::value};
	static constexpr size_t numberOfTextureIds{TextureIdsIndexITypeList::size};

	template <size_t Index>
	void AddAttributesAtIndex
	(
		const typename TypeAt<Index, VertexAttributes...>::type::type& fromBottomLeft, 
		const typename TypeAt<Index, VertexAttributes...>::type::type& fromBottomRight, 
		const typename TypeAt<Index, VertexAttributes...>::type::type& fromTopRight, 
		const typename TypeAt<Index, VertexAttributes...>::type::type& fromTopLeft
	)
	{
		static_assert(Index > 0, "The first index is reserved for the vertex's draw order attribute. Use SetDrawOrder() to change it.");
		AddAttributesAtIndex<Index>(fromBottomLeft, fromBottomRight, fromTopRight, fromTopLeft, BottomLeft, BottomRight, TopRight, TopLeft);
	}
	
	template <size_t Index>
	void GetVertexAttribute(std::array<typename TypeAt<Index, VertexAttributes...>::type::type*, 4>& outAttributes)
	{
		if constexpr (Index == 0)
		{
			outAttributes = {&BottomLeft.Value, &BottomRight.Value, &TopRight.Value, &TopLeft.Value};
		}
		else
		{
			GetVertexAttribute<Index - 1>(outAttributes, BottomLeft.Next, BottomRight.Next, TopRight.Next, TopLeft.Next);
		}
	}

	VAttrUInt::type GetDrawOrder() const
	{
		return BottomLeft.Value;
	}

	void SetDrawOrder(VAttrUInt::type drawOrder)
	{
		BottomLeft.Value = drawOrder;
		BottomRight.Value = drawOrder;
		TopRight.Value = drawOrder;
		TopLeft.Value = drawOrder;
	}

	template <class Func>
	void IterateOnTextureIds(Func function)
	{
		if constexpr (TextureIdsIndexITypeList::size == 0)
		{
			return;
		}
		else
		{
			static constexpr size_t textureIdAttributeId = ValueAt<0, TextureIdsIndexITypeList>::value;
			std::array<typename TypeAt<textureIdAttributeId, VertexAttributes...>::type::type*, 4> textureIds;
			GetVertexAttribute<textureIdAttributeId>(textureIds);
			if (*textureIds[0] != -1)
			{
				std::invoke(function, textureIds);
			}
			IterateOnTextureIds<1>(function);
		}
	}

	Vertex BottomLeft;
	Vertex BottomRight;
	Vertex TopRight;
	Vertex TopLeft;
private:
	template <size_t Index, class FromType, class TargetType>
	void AddAttributesAtIndex(const FromType& fromBottomLeft, const FromType& fromBottomRight, const FromType& fromTopRight, const FromType& fromTopLeft,
								TargetType& targetBottomLeft, TargetType& targetBottomRight, TargetType& targetTopRight, TargetType& targetTopLeft)
	{
		if constexpr (Index == 0)
		{
			targetBottomLeft.Value = fromBottomLeft;
			targetBottomRight.Value = fromBottomRight;
			targetTopRight.Value = fromTopRight;
			targetTopLeft.Value = fromTopLeft;
		}
		else
		{
			AddAttributesAtIndex<Index - 1>(fromBottomLeft, fromBottomRight, fromTopRight, fromTopLeft, targetBottomLeft.Next, targetBottomRight.Next, targetTopRight.Next, targetTopLeft.Next);
		}
	}

	template <size_t Index, class ArrayType, class CurrentType>
	void GetVertexAttribute(std::array<ArrayType*, 4>& outAttributes, CurrentType& bottomLeft, CurrentType& bottomRight, CurrentType& topRight, CurrentType& topLeft)
	{
		if constexpr (Index == 0)
		{
			outAttributes = {&bottomLeft.Value, &bottomRight.Value, &topRight.Value, &topLeft.Value};
		}
		else
		{
			GetVertexAttribute<Index - 1>(outAttributes, bottomLeft.Next, bottomRight.Next, topRight.Next, topLeft.Next);
		}
	}

	template <size_t Index, class Func>
	void IterateOnTextureIds(Func function)
	{
		if constexpr (Index >= TextureIdsIndexITypeList::size)
		{
			return;
		}
		else
		{
			static constexpr size_t textureIdAttributeId{ValueAt<Index, TextureIdsIndexITypeList>::value};
			std::array<typename TypeAt<textureIdAttributeId, VertexAttributes...>::type::type*, 4> textureIds;
			GetVertexAttribute<textureIdAttributeId>(textureIds);
			if (*textureIds[0] != -1) 
			{
				std::invoke(function, textureIds);
			}
			IterateOnTextureIds<Index + 1>(function);
		}
	}
};

template <class RenderObjectType>
struct RenderObjectComparator
{
	bool operator()(const RenderObjectType& a, const RenderObjectType& b) const
	{
		return a.GetDrawOrder() < b.GetDrawOrder();
	}
};

}
