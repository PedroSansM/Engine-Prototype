#pragma once

#include "DommusCore.h"



namespace Game
{
	class VenusComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::VenusComponent>
{
	ConstructorArgs()
		:
		VenusLinearVelocity(5.0f),
		VenusAngularVelocity(5.0f),
		TimePursuingPlayer(5.0f),
		LifeTime(10.0f),
		Health(2)
	{}
	
	float VenusLinearVelocity;
	float VenusAngularVelocity;
	float TimePursuingPlayer;
	float LifeTime;
	DCore::DUInt Health;
	DCore::DSoundEventInstance DefeatedSound;
};
#pragma pack(pop)

namespace Game
{

class VenusComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_venusLinearVelocity{0};
	static constexpr size_t a_venusAngularVelocity{1};
	static constexpr size_t a_timePursuingPlayer{2};
	static constexpr size_t a_lifeTime{3};
	static constexpr size_t a_health{4};
	static constexpr size_t a_defeatedSound{5};
public:
	VenusComponent(const DCore::ConstructorArgs<VenusComponent>&);
	~VenusComponent();
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void PhysicsUpdate(float deltaTime) override;
	virtual void OnOverlapBegin(DCore::EntityRef) override;
public:
	void HandleBurstBeginResponse();
	void HandleBurstReleaseResponse();
	void HandleShowCreatureResponse();
	void HandleVinesReleaseResponse();
	void HandleDeathEndResponse();
private:
	// Editor
	float m_venusLinearVelocity;
	float m_venusAngularVelocity;
	float m_timerPursuingPlayer;
	float m_lifeTime;
	DCore::DUInt m_health;
	DCore::DSoundEventInstance m_defeatedSound;
	// Runtime
	DCore::ComponentRef<DCore::Component> m_seedComponent;
	DCore::ComponentRef<DCore::Component> m_burstComponent;
	DCore::ComponentRef<DCore::Component> m_vinesComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_venusSpriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_venusAsmComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_venusBoxColliderComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_venusHurtBoxComponent;
	DCore::ComponentRef<DCore::TransformComponent> m_venusTransformComponent;
	DCore::ComponentRef<DCore::TransformComponent> m_playerTransformComponent;
	DCore::EntityRef m_venusEntity;
	size_t m_venusBirthAnimationParameter;
	size_t m_venusBiteAnimationParameter;
	size_t m_venusDeathAnimationParameter;
	size_t m_venusResetAnimationPanemeter;
	size_t m_shotOverlapBeginRegistrationIndex;
	DCore::DBodyId m_venusHurtBoxBodyId;
	bool m_isBorn;
	float m_currentTimePursuingPlayer;
	float m_currentLifeTime;
	float m_currentHealth;
};

class VenusComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~VenusComponentScriptComponentFormGenerator() = default;
private:
	VenusComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"VenusComponent",
			DCore::ComponentId::GetId<VenusComponent>(),
			sizeof(VenusComponent),
			sizeof(DCore::ConstructorArgs<VenusComponent>),
			{{DCore::AttributeName("Venus Linear Velocity"), DCore::AttributeType::Float, VenusComponent::a_venusLinearVelocity},
			{DCore::AttributeName("Venus Angular Velocity"), DCore::AttributeType::Float, VenusComponent::a_venusAngularVelocity},
			{DCore::AttributeName("Time Pursuing Player"), DCore::AttributeType::Float, VenusComponent::a_timePursuingPlayer},
			{DCore::AttributeName("Life Time"), DCore::AttributeType::Float, VenusComponent::a_lifeTime},
			{DCore::AttributeName("Health"), DCore::AttributeType::UInteger, VenusComponent::a_health},
			{DCore::AttributeName("Defeated Sound"), DCore::AttributeType::SoundEventInstance, VenusComponent::a_defeatedSound} },
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) VenusComponent(*static_cast<const DCore::ConstructorArgs<VenusComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<VenusComponent*>(componentAddress)->~VenusComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<VenusComponent> m_defaultArgs;
private:
	static VenusComponentScriptComponentFormGenerator s_generator;
};

}
	