#pragma once

#include "AssetManagerTypes.h"
#include "SerializationTypes.h"
#include "AssetManager.h"
#include "Component.h"
#include "Quad.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"
#include "DCoreAssert.h"
#include "ReadWriteLockGuard.h"
#include "SpriteMaterial.h"
#include "TemplateUtils.h"
#include "UUID.h"
#include "Array.h"

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <iomanip>
#include <vector>
#include <atomic>
#include <iostream>



namespace DCore
{

class SpriteComponent;

#pragma pack(push, 1)
template <>
struct ConstructorArgs<SpriteComponent>
{
	ConstructorArgs()
		:
		FlipX(false),
		FlipY(false),
		SpriteIndex(0),
		NumberOfSprites(1, 1),
		DrawOrder(0),
		PixelsPerUnit(1),
		TintColor(1.0f, 1.0f, 1.0f, 1.0f),
		Enabled(true)
	{}

	ConstructorArgs(
			DLogic flipX, 
			DLogic flipY, 
			DUInt spriteIndex, 
			DVec2 numberOfSprites, 
			DUInt drawOrder, 
			DUInt pixelsPerUnit, 
			DVec4 tintColor, 
			SpriteMaterialRef spriteMaterialRef, 
			DLogic enabled)
		:
		FlipX(flipX),
		FlipY(flipY),
		SpriteIndex(spriteIndex),
		NumberOfSprites(numberOfSprites),
		DrawOrder(drawOrder),
		PixelsPerUnit(pixelsPerUnit),
		TintColor(tintColor),
		SpriteMaterialRef(spriteMaterialRef),
		Enabled(enabled)
	{}

	DLogic FlipX;
	DLogic FlipY;
	DUInt SpriteIndex;
	DVec2 NumberOfSprites;
	DUInt DrawOrder;
	DUInt PixelsPerUnit;
	DVec4 TintColor;
	DCore::SpriteMaterialRef SpriteMaterialRef;
	DLogic Enabled;
};
#pragma pack(pop)

class SpriteComponent : public Component
{
public:
	static constexpr AttributeIdType a_flipX{0};
	static constexpr AttributeIdType a_flipY{1};
	static constexpr AttributeIdType a_spriteIndex{2};
	static constexpr AttributeIdType a_numberOfSprites{3};
	static constexpr AttributeIdType a_drawOrder{4};
	static constexpr AttributeIdType a_pixelsPerUnit{5};
	static constexpr AttributeIdType a_tintColor{6};
	static constexpr AttributeIdType a_spriteMaterial{7};
	static constexpr AttributeIdType a_enabled{8};
public:
	static constexpr size_t maxNumberOfSpriteQuads{2048};
public:
	SpriteComponent(const ConstructorArgs<SpriteComponent>& args);
	~SpriteComponent();
public:
	virtual void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint) override;
public:
	void FillSpriteQuads();
public:
	void* GetAttributePtr(AttributeIdType attributeId) override
	{
		switch (attributeId)
		{
		case a_flipX:
			return &m_flipX;
		case a_flipY:
			return &m_flipY;
		case a_spriteIndex:
			return &m_spriteIndex;
		case a_numberOfSprites:
			return &m_numberOfSprites;
		case a_drawOrder:
			return &m_drawOrder;
		case a_pixelsPerUnit:
			return &m_pixelsPerUnit;
		case a_tintColor:
			return &m_tintColor;
		case a_spriteMaterial:
			return &m_spriteMaterialRef;
		case a_enabled:
			return &m_enabled;
		default:
			return nullptr;
		}
	}
public:
	DLogic GetFlipX() const
	{
		return m_flipX;
	}

	DLogic GetFlipY() const
	{
		return m_flipY;
	}

	DUInt GetSpriteIndex() const
	{
		return m_spriteIndex;
	}

	DUInt GetDrawOrder() const
	{
		return m_drawOrder;
	}

	bool HaveQuads() const
	{
		return m_spriteQuads.Size() > 0;
	}

	Quad2 GetCurrentSpriteUvs()
	{
		DASSERT_E(HaveQuads());
		Quad2 currentSpriteUvs(m_spriteQuads[m_spriteIndex]);
		DVec2 diffuseMapSizes(GetDiffuseMapSizes());
		currentSpriteUvs.BottomLeft.x /= diffuseMapSizes.x;
		currentSpriteUvs.BottomLeft.y /= diffuseMapSizes.y;
		currentSpriteUvs.BottomRight.x /= diffuseMapSizes.x;
		currentSpriteUvs.BottomRight.y /= diffuseMapSizes.y;
		currentSpriteUvs.TopRight.x /= diffuseMapSizes.x;
		currentSpriteUvs.TopRight.y /= diffuseMapSizes.y;
		currentSpriteUvs.TopLeft.x /= diffuseMapSizes.x;
		currentSpriteUvs.TopLeft.y /= diffuseMapSizes.y;
		return currentSpriteUvs;
	}

	Quad2 GetCurrentSpriteVertexPositions()
	{
		DASSERT_E(HaveQuads());
		// Making the pivot in the center for now.
		Quad2 currentVertexPositions;
		const DVec2 diffuseMapSizes(GetDiffuseMapSizes());
		const DVec2 absoluteSizes(diffuseMapSizes.x / (2 * m_numberOfSprites.x * m_pixelsPerUnit), diffuseMapSizes.y / (2 * m_numberOfSprites.y * m_pixelsPerUnit));
		currentVertexPositions.BottomLeft = {-absoluteSizes.x, -absoluteSizes.y};
		currentVertexPositions.BottomRight = {absoluteSizes.x, -absoluteSizes.y};
		currentVertexPositions.TopRight = {absoluteSizes.x, absoluteSizes.y};
		currentVertexPositions.TopLeft = {-absoluteSizes.x, absoluteSizes.y};
		return currentVertexPositions;
	}
	
	const DVec4 GetDiffuseColor()
	{
	 	static DVec4 defaultColor(1.0f, 0.0f, 1.0f, 1.0f);
		if (!m_spriteMaterialRef.IsValid())
		{
			return defaultColor;
		}
		if (!m_spriteMaterialRef.GetDiffuseMapRef().IsValid())
		{
			return m_spriteMaterialRef.GetDiffuseColor();
		}
		// In this case, the returned color doesnt matter, because the diffuse texture in the sprite material should be sampled.
		return defaultColor;
	}

	const DVec4& GetTintColor() const
	{
		return m_tintColor;
	}
		
	void SetTintColor(const DVec4& color)
	{
		m_tintColor = color;
	}

	SpriteMaterialRef GetSpriteMaterial()
	{
		return m_spriteMaterialRef;
	}

	const SpriteMaterialRef GetSpriteMaterial() const
	{
		return m_spriteMaterialRef;
	}

	DLogic IsEnabled() const
	{
		return m_enabled;
	}

	void SetEnabled(DLogic value)
	{
		m_enabled = value;
	}

	// Use in runtime only!!
	void SetSpriteMaterial(SpriteMaterialRef spriteMaterial)
	{
		m_spriteMaterialRef = spriteMaterial;
		FillSpriteQuads();
	}

	void SetNumberOfSprites(const DVec2& numberOfSprites)
	{
		 m_numberOfSprites = numberOfSprites;
		 FillSpriteQuads();
	}

	void SetSpriteMaterialAndNumberOfSprites(SpriteMaterialRef spriteMaterial, const DVec2& numberOfSprites)
	{
		m_spriteMaterialRef = spriteMaterial;
		m_numberOfSprites = numberOfSprites;
		FillSpriteQuads();
	}

	void SetSpriteIndex(size_t index)
	{
		m_spriteIndex = index;
		if (size_t numberOfSprites(m_spriteQuads.Size()); m_spriteIndex >= numberOfSprites)
		{
			if (numberOfSprites == 0)
			{
				m_spriteIndex = 0;
				return;
			}
			m_spriteIndex = numberOfSprites - 1;
		}
	}
private:
	DLogic m_flipX;
	DLogic m_flipY;
	DUInt m_spriteIndex;
	DVec2 m_numberOfSprites;
	DUInt m_drawOrder;
	DUInt m_pixelsPerUnit;
	DVec4 m_tintColor;
	SpriteMaterialRef m_spriteMaterialRef;
	DLogic m_enabled;
	Array<Quad2, maxNumberOfSpriteQuads> m_spriteQuads;
private:
	DVec2 GetDiffuseMapSizes();
};

class SpriteComponentFormGenerator : public ComponentFormGenerator
{
public:
	~SpriteComponentFormGenerator() = default;
private:
	SpriteComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<SpriteComponent>(),
				"Sprite Component",
				false,
				sizeof(SpriteComponent),
				sizeof(ConstructorArgs<SpriteComponent>),
				{{AttributeName("Flip X"), AttributeType::Logic, SpriteComponent::a_flipX}, 
				{AttributeName("Flip Y"), AttributeType::Logic, SpriteComponent::a_flipY}, 
				{AttributeName("Sprite Index"), AttributeType::UInteger, SpriteComponent::a_spriteIndex}, 
				{AttributeName("Number of Sprites#Coluns#Rows"), AttributeType::UIVector2, SpriteComponent::a_numberOfSprites}, 
				{AttributeName("Draw Order"), AttributeType::UInteger, SpriteComponent::a_drawOrder}, 
				{AttributeName("Pixels Per Unit"), AttributeType::UInteger, SpriteComponent::a_pixelsPerUnit}, 
				{AttributeName("Tint#R#G#B#A"), AttributeType::Color, SpriteComponent::a_tintColor}, 
				{AttributeName("Sprite Material"), AttributeType::SpriteMaterial, SpriteComponent::a_spriteMaterial},
				{AttributeName("Enabled"), AttributeType::Logic, SpriteComponent::a_enabled}},
				[](void* componentAddress, const void* args) -> void
				{
					new (componentAddress) SpriteComponent(*static_cast<const ConstructorArgs<SpriteComponent>*>(args));
				},
				[](void* componentAddress) -> void
				{
					static_cast<SpriteComponent*>(componentAddress)->~SpriteComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<SpriteComponent> m_defaultArgs;
private:
	static SpriteComponentFormGenerator s_generator;
};

template <>
class ComponentRef<SpriteComponent> 
{
public:
	ComponentRef() = default;
	ComponentRef(Entity entity, InternalSceneRefType internalSceneRef, LockData& lockData)
		:
		m_entity(entity),
		m_internalSceneRef(internalSceneRef),
		m_lockData(&lockData)
	{}
	~ComponentRef() = default;
public:
	void GetAttrbutePtr(AttributeIdType attributeId, void* out, size_t attributeSize)
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		std::memcpy(out, spriteComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.OnAttributeChange(attributeId, newValue, typeHint);
	}	

	bool IsValid() const
	{
		return m_lockData != nullptr && m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<SpriteComponent>(m_entity);
	}

	DLogic GetFlipX()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::ReadLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetFlipX();
	}

	DLogic GetFlipY()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetFlipY();
	}

	DUInt GetSpriteIndex()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetSpriteIndex();
	}
	
	DUInt GetDrawOrder()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetDrawOrder();
	}

	bool HaveQuads()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.HaveQuads();
	}

	Quad2 GetCurrentSpriteUvs()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetCurrentSpriteUvs();
	}

	Quad2 GetCurrentSpriteVertexPositions()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetCurrentSpriteVertexPositions();
	}

	DVec4 GetDiffuseColor()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetDiffuseColor();
	}

	DVec4 GetTintColor()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetTintColor();
	}

	void SetTintColor(const DVec4& color)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.SetTintColor(color);
	}

	SpriteMaterialRef GetSpriteMaterialRef()
	{
		DASSERT_E(IsValid());
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.GetSpriteMaterial();
	}

	void SetSpriteMaterial(SpriteMaterialRef spriteMaterial)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.SetSpriteMaterial(spriteMaterial);
	}

	void SetNumberOfSprites(const DVec2& numberOfSprites)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.SetNumberOfSprites(numberOfSprites);	
	}

	void SetSpriteMaterialAndNumberOfSprites(SpriteMaterialRef spriteMaterial, const DVec2& numberOfSprites)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.SetSpriteMaterialAndNumberOfSprites(spriteMaterial, numberOfSprites);	
	}

	void SetSpriteIndex(size_t index)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.SetSpriteIndex(index);	
	}

	DLogic IsEnabled() const
	{
		DASSERT_E(IsValid());
		const SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		return spriteComponent.IsEnabled();
	}

	void SetEnabled(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.SetEnabled(value);
	}
	 
	void FillSpriteQuads()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard sceneGuard(LockType::WriteLock, *m_lockData);
		ReadWriteLockGuard materialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
		ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
		SpriteComponent& spriteComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<SpriteComponent>(m_entity));
		spriteComponent.FillSpriteQuads();
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
