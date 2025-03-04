#pragma once

#include "DommusCore.h"



namespace Game
{
	class AcornComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::AcornComponent>
{
	ConstructorArgs()
		:
		InitialVelocity(10.0f),
		Acceleration(1.0f),
		LifeTime(1.0f)
	{}
	
	float InitialVelocity;
	float Acceleration;
	float LifeTime;
};
#pragma pack(pop)

namespace Game
{

class AcornComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_initialVelocity{0};
	static constexpr size_t a_acceleration{1};
	static constexpr size_t a_lifeTime{2};
public:
	AcornComponent(const DCore::ConstructorArgs<AcornComponent>&);
	~AcornComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void PhysicsUpdate(float physicsDeltaTime) override;
public:
	void Enable();
	void Launch();
private:
	// Editor
	float m_initialVelocity;
	float m_acceleration;
	float m_lifeTime;
	size_t m_acornTypeIndex;
	// Runtime
	DCore::ComponentRef<DCore::Component> m_playerTransformComponent;
	DCore::ComponentRef<DCore::TransformComponent> m_transformComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_boxColliderComponent;
	size_t m_spinAnimationParameterIndex;
	DCore::DVec3 m_initialPosition;
	DCore::DVec3 m_direction;
	float m_currentLifeTime;
	bool m_launched;
private:
	void GetAnimationParametersIndexes();
};

class AcornComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~AcornComponentScriptComponentFormGenerator() = default;
private:
	AcornComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"AcornComponent",
			DCore::ComponentId::GetId<AcornComponent>(),
			sizeof(AcornComponent),
			sizeof(DCore::ConstructorArgs<AcornComponent>),
			{{DCore::AttributeName("Initial Velocity"), DCore::AttributeType::Float, AcornComponent::a_initialVelocity},
			{DCore::AttributeName("Acceleration"), DCore::AttributeType::Float, AcornComponent::a_acceleration},
			{DCore::AttributeName("Life Time"), DCore::AttributeType::Float, AcornComponent::a_lifeTime}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) AcornComponent(*static_cast<const DCore::ConstructorArgs<AcornComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<AcornComponent*>(componentAddress)->~AcornComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<AcornComponent> m_defaultArgs;
private:
	static AcornComponentScriptComponentFormGenerator s_generator;
};

}
	
