#pragma once

#include "DommusCore.h"



namespace Game
{
	class BurstComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::BurstComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class BurstComponent : public DCore::ScriptComponent
{
public:
	BurstComponent(const DCore::ConstructorArgs<BurstComponent>&)
	{}
	~BurstComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start();
	virtual void OnAnimationEvent(size_t metachannelId) override;
public:
	void BeginBurst();
private:
	DCore::ComponentRef<DCore::Component> m_burstable;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	size_t m_burstAnimationParameter;
};

class BurstComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~BurstComponentScriptComponentFormGenerator() = default;
private:
	BurstComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"BurstComponent",
			DCore::ComponentId::GetId<BurstComponent>(),
			sizeof(BurstComponent),
			sizeof(DCore::ConstructorArgs<BurstComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) BurstComponent(*static_cast<const DCore::ConstructorArgs<BurstComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<BurstComponent*>(componentAddress)->~BurstComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<BurstComponent> m_defaultArgs;
private:
	static BurstComponentScriptComponentFormGenerator s_generator;
};

}
	