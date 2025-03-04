#include "SpriteSheetGen.h"
#include "TextureManager.h"

#include <tuple>



namespace DEditor
{

SpriteSheetGen::SpriteSheetGen(const uuidType& uuid, const stringType& name)
	:
	m_uuid(uuid),
	m_name(name),
	m_numberOfColumns(1)
{}

SpriteSheetGen::SpriteSheetGen(SpriteSheetGen&& other) noexcept
	:
	m_uuid(other.m_uuid),
	m_name(std::move(other.m_name)),
	m_scene(std::move(other.m_scene)),
	m_spriteSheetElements(std::move(other.m_spriteSheetElements)),
	m_numberOfColumns(other.m_numberOfColumns)
{}

SpriteSheetGen::~SpriteSheetGen()
{
	DCore::Registry& registry(m_scene.GetRegistry());
	registry.IterateOnEntities
	(
		[&](DCore::Entity entity) -> bool
		{
			registry.DestroyEntity
			(
				entity, 
				[&](DCore::ComponentIdType componentId, void* component) -> void
				{
					 if (componentId == DCore::ComponentId::GetId<DCore::TransformComponent>())
					 {
						static_cast<DCore::TransformComponent*>(component)->DCore::TransformComponent::~TransformComponent();
					 }
					 else if (componentId == DCore::ComponentId::GetId<DCore::SpriteComponent>())
					 {
						static_cast<DCore::SpriteComponent*>(component)->DCore::SpriteComponent::~SpriteComponent();
					 }
				}
			);
			return false;
		}
	);
}

size_t SpriteSheetGen::AddSprite(const uuidType& textureUUID)
{
	DCore::Texture2DRef diffuseTexture(TextureManager::Get().LoadTexture2D(textureUUID));
	DASSERT_E(diffuseTexture.IsValid());
	DCore::Registry& registry(m_scene.GetRegistry());
	spriteMaterialType spriteMaterial(DCore::SpriteMaterialType::Unlit);
	spriteMaterial.SetDiffuseMapRef(diffuseTexture);
	uuidType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	DCore::SpriteMaterialRef spriteMaterialRef(DCore::AssetManager::Get().LoadSpriteMaterial(uuid, std::move(spriteMaterial)));
	DCore::ConstructorArgs<DCore::SpriteComponent> spriteComponentArgs;
	spriteComponentArgs.DrawOrder = 1;
	spriteComponentArgs.SpriteMaterialRef = spriteMaterialRef;
	DCore::Entity entity(registry.CreateEntity<DCore::TransformComponent, DCore::SpriteComponent>(std::make_tuple(DCore::ConstructorArgs<DCore::TransformComponent>()), std::make_tuple(spriteComponentArgs)));
	m_spriteSheetElements.push_back({entity});
	return m_spriteSheetElements.size() - 1;
}

void SpriteSheetGen::RemoveSprite(size_t index)
{
	DASSERT_E(index < m_spriteSheetElements.size());
	m_spriteSheetElements.erase(m_spriteSheetElements.begin() + index);
}

void SpriteSheetGen::UpdateSpriteIndex(size_t currentIndex, size_t newIndex)
{
	DASSERT_E(currentIndex < m_spriteSheetElements.size() && newIndex < m_spriteSheetElements.size());
	const SpriteSheetElement temp(m_spriteSheetElements[currentIndex]);
	m_spriteSheetElements[currentIndex] = m_spriteSheetElements[newIndex];
	m_spriteSheetElements[newIndex] = temp;
}

void SpriteSheetGen::UpdateSpriteOffset(size_t spriteIndex, const dVec2& newPosition)
{
	DASSERT_E(spriteIndex < m_spriteSheetElements.size());
	DCore::TransformComponent& transformComponent(m_scene.GetRegistry().GetComponents<DCore::TransformComponent>(m_spriteSheetElements[spriteIndex].Entity));
	transformComponent.SetTranslation({newPosition.x, newPosition.y, 0.0f});
}

SpriteSheetGen::dVec2 SpriteSheetGen::GetSpriteOffset(size_t index) const
{
	DASSERT_E(index < m_spriteSheetElements.size());
	const DCore::Registry& registry(m_scene.GetRegistry());
	const DCore::TransformComponent& transformComponent(registry.GetComponents<DCore::TransformComponent>(m_spriteSheetElements[index].Entity));
	const DCore::DVec3& translation(transformComponent.GetTranslation());
	return {translation.x, translation.y};
}

}
