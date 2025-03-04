#pragma once

#include "DommusCore.h"



namespace Game
{
	class PlatformComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::PlatformComponent>
{
	ConstructorArgs()
	{}

	float InitialCoefficient;
	float AltitudeVariation;
	float MovementPeriod;
	float MinimalAltitude;
	float TransitionVelocity;
};
#pragma pack(pop)

namespace Game
{

class PlatformComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_initialCoefficientId{0};
	static constexpr size_t a_altitudeVariationId{1};
	static constexpr size_t a_movementPeriodId{2};
	static constexpr size_t a_minimalAltitudeId{3};
	static constexpr size_t a_transitionVelocityId{4};
public:
	PlatformComponent(const DCore::ConstructorArgs<PlatformComponent>&);
	~PlatformComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void PhysicsUpdate(float physicsDeltaTime) override;
public:
	void BeginPlayerCollision()
	{
		m_state = PlatformState::Transition;
	}
	void EndPlayerCollision()
	{
		m_state = PlatformState::NotColliding;
	}
private:
	enum class PlatformState
	{
		NotColliding,
		Transition
	};
private:
	// Editor
	float m_initialCoefficient;
	float m_altitudeVariation;
	float m_movementPeriod;
	float m_minimalAltitude;
	float m_transitionVelocity;
	//
	PlatformState m_state;
	float m_currentCoefficient;
	float m_halfAltitudeVariation;
	float m_translationOffset;
	DCore::ComponentRef<DCore::TransformComponent> m_transform;
private:
	void HandleNotCollidingState(float deltaTime);
	void HandleTransitionState(float deltaTime);
};

class PlatformComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~PlatformComponentScriptComponentFormGenerator() = default;
private:
	PlatformComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"PlatformComponent",
			DCore::ComponentId::GetId<PlatformComponent>(),
			sizeof(PlatformComponent),
			sizeof(DCore::ConstructorArgs<PlatformComponent>),
			{{DCore::AttributeName("Initial Coefficient"), DCore::AttributeType::Float, PlatformComponent::a_initialCoefficientId},
			{DCore::AttributeName("Altitude Variation"), DCore::AttributeType::Float, PlatformComponent::a_altitudeVariationId},
			{DCore::AttributeName("Movement Period"), DCore::AttributeType::Float, PlatformComponent::a_movementPeriodId},
			{DCore::AttributeName("Minimal Altitude"), DCore::AttributeType::Float, PlatformComponent::a_minimalAltitudeId},
			{DCore::AttributeName("Transition Velocity"), DCore::AttributeType::Float, PlatformComponent::a_transitionVelocityId}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) PlatformComponent(*static_cast<const DCore::ConstructorArgs<PlatformComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<PlatformComponent*>(componentAddress)->~PlatformComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<PlatformComponent> m_defaultArgs;
private:
	static PlatformComponentScriptComponentFormGenerator s_generator;
};

}
	
