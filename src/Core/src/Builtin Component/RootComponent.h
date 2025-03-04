#pragma once

#include "AssetManagerTypes.h"
#include "Component.h"
#include "ComponentForm.h"
#include "ComponentId.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"

#include <atomic>



namespace DCore
{

class RootComponent : public Component
{
public:
	RootComponent() = default;
	~RootComponent() = default;
};

class RootComponentFormGenerator : private ComponentFormGenerator
{
public:
	~RootComponentFormGenerator() = default;
private:
	RootComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<RootComponent>(),
				"Root component",
				false,
				sizeof(RootComponent),
				0,
				{},
				[](void* componentAddress, const void* args) -> void
				{
					new (componentAddress) RootComponent();
				},
				[](void* componentAddress) -> void
				{},
				nullptr
			}
		)
	{}
private:
	static RootComponentFormGenerator s_generator;
};

template <>
class ComponentRef<RootComponent>
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
