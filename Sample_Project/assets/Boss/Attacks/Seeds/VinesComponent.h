#pragma once

#include "DommusCore.h"



namespace Game
{
	class VinesComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::VinesComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class VinesComponent : public DCore::ScriptComponent
{
public:
	VinesComponent(const DCore::ConstructorArgs<VinesComponent>&);
	~VinesComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override;
	virtual void OnAnimationEvent(size_t meachannelId) override;
public:
	void BeginGrow();
private:
	DCore::ComponentRef<DCore::Component> m_burstable;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	size_t m_growAnimationParameter;
};

class VinesComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~VinesComponentScriptComponentFormGenerator() = default;
private:
	VinesComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"VinesComponent",
			DCore::ComponentId::GetId<VinesComponent>(),
			sizeof(VinesComponent),
			sizeof(DCore::ConstructorArgs<VinesComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) VinesComponent(*static_cast<const DCore::ConstructorArgs<VinesComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<VinesComponent*>(componentAddress)->~VinesComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<VinesComponent> m_defaultArgs;
private:
	static VinesComponentScriptComponentFormGenerator s_generator;
};

}
	