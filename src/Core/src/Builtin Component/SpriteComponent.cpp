#include "SpriteComponent.h"
#include "SpriteMaterial.h"
#include "TemplateUtils.h"

#include <algorithm>



namespace DCore
{

SpriteComponent::SpriteComponent(const ConstructorArgs<SpriteComponent>& args)
	:
	m_flipX(args.FlipX),
	m_flipY(args.FlipY),
	m_spriteIndex(args.SpriteIndex),
	m_numberOfSprites(args.NumberOfSprites),
	m_drawOrder(args.DrawOrder),
	m_pixelsPerUnit(args.PixelsPerUnit),
	m_tintColor(args.TintColor),
	m_spriteMaterialRef(args.SpriteMaterialRef),
	m_enabled(args.Enabled)
{
	FillSpriteQuads();
}

SpriteComponent::~SpriteComponent()
{
	m_spriteMaterialRef.Unload();
}

void SpriteComponent::OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_flipX:
		m_flipX = *static_cast<DLogic*>(newValue);
		return;
	case a_flipY:
		m_flipY = *static_cast<DLogic*>(newValue);
		return;
	case a_spriteIndex:
		SetSpriteIndex(typeHint == AttributeType::Integer ? m_spriteIndex = *static_cast<DInt*>(newValue) : *static_cast<DUInt*>(newValue));
		return;
	case a_numberOfSprites:
		m_numberOfSprites = *static_cast<DVec2*>(newValue);
		m_numberOfSprites.x = (std::max)(m_numberOfSprites.x, static_cast<decltype(m_numberOfSprites.x)>(1));
		m_numberOfSprites.y = (std::max)(m_numberOfSprites.y, static_cast<decltype(m_numberOfSprites.y)>(1));
		m_spriteIndex = (std::min)(m_spriteIndex , static_cast<DUInt>(m_numberOfSprites.x * m_numberOfSprites.y));
		FillSpriteQuads();
		return;
	case a_drawOrder:
		m_drawOrder = *static_cast<DUInt*>(newValue);
		return;
	case a_pixelsPerUnit:
		m_pixelsPerUnit = *static_cast<DUInt*>(newValue);
		m_pixelsPerUnit = (std::max)(static_cast<decltype(m_pixelsPerUnit)>(1), m_pixelsPerUnit);
		return;
	case a_tintColor:
		SetTintColor(*static_cast<DVec4*>(newValue));
		return;
	case a_spriteMaterial:
		m_spriteMaterialRef.Unload();
		m_spriteMaterialRef = *static_cast<SpriteMaterialRef*>(newValue);
		FillSpriteQuads();
		return;
	case a_enabled:
		m_enabled = *static_cast<DLogic*>(newValue);
		return;
	default:
		DASSERT_E(false);
		return;
	}
}

DVec2 SpriteComponent::GetDiffuseMapSizes()
{
	constexpr DVec2 defaultDiffuseMapSize({32, 32});
	if (!m_spriteMaterialRef.IsValid() || !m_spriteMaterialRef.GetDiffuseMapRef().IsValid())
	{
		return defaultDiffuseMapSize;
	}
	return m_spriteMaterialRef.GetDiffuseMapRef().GetDimensions();
}

void SpriteComponent::FillSpriteQuads()
{
	m_spriteQuads.Clear();
	DVec2 diffuseMapSizes(GetDiffuseMapSizes());
	DVec2 cellSize(diffuseMapSizes.x / m_numberOfSprites.x, diffuseMapSizes.y / m_numberOfSprites.y);
	for (int y(m_numberOfSprites.y - 1); y >= 0; y--)
	{
		for (size_t x(0); x < m_numberOfSprites.x; x++)
		{
			DVec2 bottomLeft = {x * cellSize.x, y * cellSize.y};
			DVec2 bottomRight = {bottomLeft.x + cellSize.x, bottomLeft.y};
			DVec2 topRight = {bottomRight.x, bottomRight.y + cellSize.y};
			DVec2 topLeft = {bottomLeft.x, bottomLeft.y + cellSize.y};
			m_spriteQuads.PushBack({bottomLeft, bottomRight, topRight, topLeft});
		}
	}
}

SpriteComponentFormGenerator SpriteComponentFormGenerator::s_generator;

}
