#pragma once

#include "Asset.h"
#include "Scene.h"
#include "ReadWriteLockGuard.h"
#include "DCoreAssert.h"



namespace DCore
{

class AttributeRefData
{
public:
	AttributeRefData(Entity entity, 
					InternalSceneRefType internalSceneRef, 
					ComponentIdType componentId, 
					AttributeIdType attributeId, 
					LockData& lockData)
		:
		m_entity(entity),
		m_internalSceneRef(internalSceneRef),
		m_componentId(componentId),
		m_attribureId(attributeId),
		m_lockData(&lockData)
	{}
	virtual ~AttributeRefData() = default;
protected:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	ComponentIdType m_componentId;
	AttributeIdType m_attribureId;
	LockData* m_lockData;
};

template <class T>
class AttributeRef : public AttributeRefData
{};

}
