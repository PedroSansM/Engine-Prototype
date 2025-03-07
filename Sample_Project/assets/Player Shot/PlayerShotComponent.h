#pragma once

#include "DommusCore.h"



namespace Game
{
	class PlayerShotComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::PlayerShotComponent>
{
	ConstructorArgs()
	{}

	ConstructorArgs(DCore::DSoundEventInstance hitSound)
		:
		HitSound(hitSound)
	{}

	DCore::DSoundEventInstance HitSound;
};
#pragma pack(pop)

namespace Game
{

class PlayerShotComponent : public DCore::ScriptComponent
{
public:
	PlayerShotComponent(const DCore::ConstructorArgs<PlayerShotComponent>& args)
		:
		m_isFree(true),
		m_hitSound(args.HitSound)
	{}
	~PlayerShotComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		DASSERT_E(false);
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override
	{
		auto [transformComponent, boxColliderComponent, asmComponent] = m_entityRef.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent, DCore::AnimationStateMachineComponent>();
		m_transformComponent = transformComponent;
		m_boxColliderComponent = boxColliderComponent;
		m_asmComponent = asmComponent;
		DASSERT_E(m_transformComponent.IsValid() && m_boxColliderComponent.IsValid() && m_asmComponent.IsValid());
		DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Dead", m_deadAnimationParameter));
		DASSERT_K(m_hitSound.Setup());
	}

	virtual void Update(float deltaTime) override
	{
		if (m_timeAlive > 0.0f && !m_isFree)
		{
			m_timeAlive -= deltaTime;
			if (m_timeAlive <= 0.0f)
			{
				m_isFree = true;
				m_boxColliderComponent.SetEnabled(false);
				m_transformComponent.SetTranslation({2.0f, 9.0f, 0.0f});
			}
		}
	}

	virtual void OnAnimationEvent(size_t metachannelId) override
	{
		constexpr size_t deadEndAnimationEvent(0);
		switch (deadEndAnimationEvent)
		{
		case deadEndAnimationEvent:
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_deadAnimationParameter, DCore::LogicParameter{ false });
			m_isFree = true;
			m_transformComponent.SetTranslation({2.0f, 9.0f, 0.0f});
			return;
		default:
			return;
		}
	}
public:
	void Fire(const DCore::DVec2& velocity, const DCore::DVec2& initialPosition, float rotation, bool toFlip);
	void Kill();
public:
	bool IsFree() const
	{
		return m_isFree;
	}
private:
	DCore::DSoundEventInstance m_hitSound;
	bool m_isFree;
	float m_timeAlive;
	DCore::ComponentRef<DCore::TransformComponent> m_transformComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_boxColliderComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	size_t m_deadAnimationParameter;
};

class PlayerShotComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~PlayerShotComponentScriptComponentFormGenerator() = default;
private:
	PlayerShotComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"PlayerShotComponent",
			DCore::ComponentId::GetId<PlayerShotComponent>(),
			sizeof(PlayerShotComponent),
			0,
			{}, 
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) PlayerShotComponent(*static_cast<const DCore::ConstructorArgs<PlayerShotComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<PlayerShotComponent*>(componentAddress)->~PlayerShotComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<PlayerShotComponent> m_defaultArgs;
private:
	static PlayerShotComponentScriptComponentFormGenerator s_generator;
};

}
	
