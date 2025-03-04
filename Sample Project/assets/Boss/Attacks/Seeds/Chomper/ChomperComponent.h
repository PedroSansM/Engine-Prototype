#pragma once

#include "DommusCore.h"



namespace Game
{
	class ChomperComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::ChomperComponent>
{
	ConstructorArgs()
		:
		LifeTime(10.0f),
		Health(2)
	{}

	float LifeTime;
	DCore::DUInt Health;
	DCore::DSoundEventInstance DefeatedSound;
};
#pragma pack(pop)

namespace Game
{

class ChomperComponent 
	: 
	public DCore::ScriptComponent
{
public:
	static constexpr size_t a_lifeTime{ 0 };
	static constexpr size_t a_health{ 1 };
	static constexpr size_t a_defeatedSound{ 2 };
public:
	ChomperComponent(const DCore::ConstructorArgs<ChomperComponent>&);
	~ChomperComponent();
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
	virtual void OnOverlapBegin(DCore::EntityRef) override;
public:
	void HandleBurstBeginResponse();
	void HandleBurstReleaseResponse();
	void HandleDeathEndResponse();
private:
	// Editor
	float m_lifeTime;
	DCore::DUInt m_health;
	DCore::DSoundEventInstance m_defeatedSound;
	// Runtime
	DCore::ComponentRef<DCore::Component> m_seedComponent;
	DCore::ComponentRef<DCore::Component> m_burstComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_chomperSpriteComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_chomperBoxColliderComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_chomperAsmComponent;
	DCore::ComponentRef<DCore::TransformComponent> m_chomperTransformComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_chomperHurtBoxComponent;
	size_t m_chomperBiteAnimationParameter;
	size_t m_chomperDeathAnimationParameter;
	size_t m_chomperResetAnimationParameter;
	DCore::DVec3 m_chomperRelativePosition;
	float m_currentLifeTime;
	DCore::DUInt m_currentHealth;
	size_t m_shotOverlapBeginRegistrationIndex;
	DCore::DBodyId m_chomperHurtBoxBodyId;
};

class ChomperComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~ChomperComponentScriptComponentFormGenerator() = default;
private:
	ChomperComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"ChomperComponent",
			DCore::ComponentId::GetId<ChomperComponent>(),
			sizeof(ChomperComponent),
			sizeof(DCore::ConstructorArgs<ChomperComponent>),
			{{DCore::AttributeName("Life Time"), DCore::AttributeType::Float, ChomperComponent::a_lifeTime},
			{DCore::AttributeName("Health"), DCore::AttributeType::UInteger, ChomperComponent::a_health},
			{DCore::AttributeName("Defeated Sound"), DCore::AttributeType::SoundEventInstance, ChomperComponent::a_defeatedSound}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) ChomperComponent(*static_cast<const DCore::ConstructorArgs<ChomperComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<ChomperComponent*>(componentAddress)->~ChomperComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<ChomperComponent> m_defaultArgs;
private:
	static ChomperComponentScriptComponentFormGenerator s_generator;
};

}
	