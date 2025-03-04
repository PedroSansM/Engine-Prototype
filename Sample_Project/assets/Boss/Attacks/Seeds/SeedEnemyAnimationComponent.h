#pragma once

#include "DommusCore.h"



namespace Game
{
	class SeedEnemyAnimationComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::SeedEnemyAnimationComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class SeedEnemyAnimationComponent : public DCore::ScriptComponent
{
public:
	SeedEnemyAnimationComponent(const DCore::ConstructorArgs<SeedEnemyAnimationComponent>&)
	{}
	~SeedEnemyAnimationComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override;
	virtual void OnMetachannelEvent(size_t metachannelId) override;
private:
	DCore::ComponentRef<DCore::Component> m_burstable;
};

class SeedEnemyAnimationComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~SeedEnemyAnimationComponentScriptComponentFormGenerator() = default;
private:
	SeedEnemyAnimationComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"SeedEnemyAnimationComponent",
			DCore::ComponentId::GetId<SeedEnemyAnimationComponent>(),
			sizeof(SeedEnemyAnimationComponent),
			sizeof(DCore::ConstructorArgs<SeedEnemyAnimationComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) SeedEnemyAnimationComponent(*static_cast<const DCore::ConstructorArgs<SeedEnemyAnimationComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<SeedEnemyAnimationComponent*>(componentAddress)->~SeedEnemyAnimationComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<SeedEnemyAnimationComponent> m_defaultArgs;
private:
	static SeedEnemyAnimationComponentScriptComponentFormGenerator s_generator;
};

}
	