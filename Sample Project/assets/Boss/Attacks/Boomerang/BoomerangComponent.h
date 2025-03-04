#pragma once

#include "DommusCore.h"



namespace Game
{
	class BoomerangComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::BoomerangComponent>
{
	ConstructorArgs()
	{}
	
	DCore::DSoundEventInstance Sound;
};
#pragma pack(pop)

namespace Game
{

class BoomerangComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_sound{0};
public:
	BoomerangComponent(const DCore::ConstructorArgs<BoomerangComponent>&);
	~BoomerangComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
public:
	void Enable();
public:
	void SetHalfLifeTime(float value)
	{
		m_halfLifeTime = value;
	}

	void SetVelocity(float value)
	{
		m_velocity = value;
	}

	void SetLowTranslationY(float value)
	{
		m_lowTranslationY = value;
	}

	void SetTimeToLaunch(float value)
	{
		m_timeToLaunch = value;
	}

	void SetAdditionalReturnLifeTime(float value)
	{
		m_additionalReturnLifeTime = value;
	}

	bool IsFree() const
	{
		return  m_isFree;
	}
private:
	// Editor
	DCore::DSoundEventInstance m_sound;
	// Runtime
	float m_halfLifeTime;
	float m_velocity;
	float m_lowTranslationY;
	float m_timeToLaunch;
	float m_additionalReturnLifeTime;
	float m_currentHalfLifeTime;
	float m_currentTimeToLaunch;
	bool m_halfWayCompleted;
	bool m_isFree;
	DCore::ComponentRef<DCore::TransformComponent> m_transformComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_boxColliderComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::DVec3 m_initialPosition;
private:
	void Launch();
	void ChangeDirection();
	void Disable();
};

class BoomerangComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~BoomerangComponentScriptComponentFormGenerator() = default;
private:
	BoomerangComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"BoomerangComponent",
			DCore::ComponentId::GetId<BoomerangComponent>(),
			sizeof(BoomerangComponent),
			sizeof(DCore::ConstructorArgs<BoomerangComponent>),
			{{DCore::AttributeName("Sound"), DCore::AttributeType::SoundEventInstance, BoomerangComponent::a_sound}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) BoomerangComponent(*static_cast<const DCore::ConstructorArgs<BoomerangComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<BoomerangComponent*>(componentAddress)->~BoomerangComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<BoomerangComponent> m_defaultArgs;
private:
	static BoomerangComponentScriptComponentFormGenerator s_generator;
};

}
	
