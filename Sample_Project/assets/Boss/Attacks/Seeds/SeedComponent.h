#pragma once

#include "DommusCore.h"



namespace Game
{
	class SeedComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::SeedComponent>
{
	ConstructorArgs()
		:
		SeedFallVelocity(5.0f)
	{}

	float SeedFallVelocity;
};
#pragma pack(pop)

namespace Game
{

class SeedComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_seedFallVelocity{0};
public:
	SeedComponent(const DCore::ConstructorArgs<SeedComponent>&);
	~SeedComponent();
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Awake() override;
	virtual void OnCollisionBegin(DCore::EntityRef) override;
	virtual void OnAnimationEvent(size_t matachannelId) override;
public:
	void BeginFall();
public:
	float GetSeedFallVelocity() const
	{
		return m_seedFallVelocity;
	}
private:
	// Editor
	float m_seedFallVelocity;
	// Runtime
	DCore::ComponentRef<DCore::Component> m_burstable;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_seedBoxColliderComponent;
	DCore::DBodyId m_seedBoxColliderId;
	size_t m_seedBoxColliderOnCollisionBeginRegistrationIndex;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	size_t m_plantAnimationParameter;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	
};

class SeedComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~SeedComponentScriptComponentFormGenerator() = default;
private:
	SeedComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"SeedComponent",
			DCore::ComponentId::GetId<SeedComponent>(),
			sizeof(SeedComponent),
			sizeof(DCore::ConstructorArgs<SeedComponent>),
			{ {DCore::AttributeName("Seed Fall Velocity"), DCore::AttributeType::Float, SeedComponent::a_seedFallVelocity} },
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) SeedComponent(*static_cast<const DCore::ConstructorArgs<SeedComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<SeedComponent*>(componentAddress)->~SeedComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<SeedComponent> m_defaultArgs;
private:
	static SeedComponentScriptComponentFormGenerator s_generator;
};

}
	