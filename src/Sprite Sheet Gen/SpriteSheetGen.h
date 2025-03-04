#pragma once

#include "DommusCore.h"

#include <string>
#include <vector>



namespace DEditor
{

struct SpriteSheetElement
{
	DCore::Entity Entity;
};

class SpriteSheetGen
{
public:
	using uuidType = DCore::UUIDType;
	using sceneType = DCore::Scene;
	using spriteMaterialType = DCore::SpriteMaterial;
	using dVec2 = DCore::DVec2;
	using stringType = std::string;
	using spriteSheetElementContainerType = std::vector<SpriteSheetElement>;
public:
	SpriteSheetGen(const uuidType&, const stringType& name);
	SpriteSheetGen(SpriteSheetGen&&) noexcept;
	~SpriteSheetGen();
public:
 	size_t AddSprite(const uuidType& textureUUID);
	void RemoveSprite(size_t index);
	void UpdateSpriteIndex(size_t currentIndex, size_t newIndex);
	void UpdateSpriteOffset(size_t index, const dVec2&);
	dVec2 GetSpriteOffset(size_t index) const;
public:
	const uuidType& GetUUID() const
	{
		return m_uuid;
	}

	const stringType& GetName() const
	{
		return m_name;
	}

	void SetName(const stringType& name)
	{
		m_name = name;
	}

	SpriteSheetElement GetSpriteSheetElement(size_t index) const
	{
		DASSERT_E(index < m_spriteSheetElements.size());
		return m_spriteSheetElements[index];
	}

	DCore::Registry& GetRegistry()
	{
		return m_scene.GetRegistry();
	}

	size_t GetNumberOfElements() const
	{
		return m_spriteSheetElements.size();
	}

	size_t GetNumberOfColumns() const
	{
		return m_numberOfColumns;
	}

	void SetNumberOfColumns(size_t value)
	{
		m_numberOfColumns = value;
	}
public:
	template <class Func>
	void IterateOnElements(Func function)
	{
		for (SpriteSheetElement& element : m_spriteSheetElements)
		{
			if (std::invoke(function, element))
			{
				return;
			}
		}
	}
private:
	uuidType m_uuid;
	stringType m_name;
	sceneType m_scene;
	spriteSheetElementContainerType m_spriteSheetElements;
	size_t m_numberOfColumns;
};

}
