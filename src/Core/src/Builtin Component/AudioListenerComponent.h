#pragma once

#include "TemplateUtils.h"
#include "Component.h"
#include "ComponentForm.h"
#include "ComponentId.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"



namespace DCore
{

class AudioListenerComponent;

#pragma pack(push, 1)
template<>
struct ConstructorArgs<AudioListenerComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

class AudioListenerComponent : public Component
{
public:
	AudioListenerComponent(const ConstructorArgs<AudioListenerComponent>&)
	{}
	~AudioListenerComponent() = default;
};

class AudioListenerComponentFormGenerator : public ComponentFormGenerator
{
public:
	~AudioListenerComponentFormGenerator() = default;
private:
	AudioListenerComponentFormGenerator()
		:
		ComponentFormGenerator({
			ComponentId::GetId<AudioListenerComponent>(),
			"Audio Listener Component",
			false,
			sizeof(AudioListenerComponent),
			sizeof(ConstructorArgs<AudioListenerComponent>),
			{},
			[](void* componentAddress, const void* args) -> void
			{
				new (componentAddress) AudioListenerComponent(*static_cast<const ConstructorArgs<AudioListenerComponent>*>(args));
			},
			[](void* componentAddress) -> void
			{
				static_cast<AudioListenerComponent*>(componentAddress)->~AudioListenerComponent();
			},
			&m_defaultArgs})
	{}
private:
	ConstructorArgs<AudioListenerComponent> m_defaultArgs;
private:
	static AudioListenerComponentFormGenerator s_generator;
};

template <>
class ComponentRef<AudioListenerComponent>
{
public:
	ComponentRef(Entity entity, InternalSceneRefType internalSceneRef, LockData& lockData)
	{}
	~ComponentRef() = default;
public:
	void GetAttrbutePtr(AttributeIdType, void* out, size_t attributeSize)
	{}
	
	void OnAttributeChange(AttributeIdType, void* newValue)
	{}
};

}