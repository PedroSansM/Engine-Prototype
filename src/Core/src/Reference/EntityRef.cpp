#include "EntityRef.h"
#include "Asset.h"
#include "AssetManager.h"
#include "ComponentForm.h"
#include "ComponentId.h"
#include "ECSTypes.h"
#include "DCoreMath.h"
#include "ReadWriteLockGuard.h"
#include "RootComponent.h"
#include "Scene.h"
#include "ChildrenComponent.h"
#include "ChildComponent.h"
#include "NameComponent.h"
#include "SerializationTypes.h"
#include "TemplateUtils.h"
#include "UUIDComponent.h"
#include "DCoreAssert.h"
#include "TransformComponent.h"



namespace DCore
{

EntityRef::EntityRef()
	:
	m_lockData(nullptr)
{}

EntityRef::EntityRef(Entity entity, SceneRef sceneRef)
	:
	m_entity(entity),
	m_internalSceneRef(sceneRef.GetInternalSceneRef()),
	m_lockData(sceneRef.GetLockData())
{}

EntityRef::EntityRef(const EntityRef& other)
	:
	m_entity(other.m_entity),
	m_internalSceneRef(other.m_internalSceneRef),
	m_lockData(other.m_lockData)
{}

bool EntityRef::IsValid() const
{
	return m_lockData != nullptr && m_internalSceneRef.IsValid() && m_entity.IsValid();
}

void EntityRef::SetParent(EntityRef parentRef)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	if (*this == parentRef || HaveChildRecursive(parentRef))
	{
		return;
	}
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (!registry.HaveComponents<ChildComponent>(m_entity))
	{
		if (registry.HaveComponents<RootComponent>(m_entity))
		{
			registry.RemoveComponents<RootComponent>(m_entity);
		}
		ConstructorArgs<ChildComponent> args(parentRef);
		registry.AddComponents<ChildComponent>(m_entity, std::make_tuple(args));
	}
	else
	{
		ChildComponent& childComponent(registry.GetComponents<ChildComponent>(m_entity));
		EntityRef oldParentRef(childComponent.GetParent());
		if (oldParentRef == parentRef)
		{
			return;
		}
		oldParentRef.RemoveChild(*this, false);
		childComponent.RemoveNext();
		childComponent.RemovePrevious();
		childComponent.SetParent(parentRef);
	}
	if (!registry.HaveComponents<ChildrenComponent>(parentRef.m_entity))
	{
		const ConstructorArgs<ChildrenComponent> args;
		registry.AddComponents<ChildrenComponent>(parentRef.m_entity, std::make_tuple(args));
	}
	ChildrenComponent& parentChildrenComponent(registry.GetComponents<ChildrenComponent>(parentRef.m_entity));
	parentChildrenComponent.AddChild(*this);
}

bool EntityRef::TryGetParent(EntityRef& outParent)
{
	DASSERT_E(IsValid());
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (!registry.HaveComponents<ChildComponent>(m_entity))
	{
		return false;
	}
	outParent = registry.GetComponents<ChildComponent>(m_entity).GetParent();
	return true;
}

bool EntityRef::HaveParent() const
{
	DASSERT_E(IsValid());
	return m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<ChildComponent>(m_entity);
}

size_t EntityRef::GetNumberOfChildren() const
{
	DASSERT_E(IsValid());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (registry.HaveComponents<ChildrenComponent>(m_entity))
	{
		return registry.GetComponents<ChildrenComponent>(m_entity).GetNumberOfChildren();
	}
	return 0;
}

void EntityRef::RemoveChild(EntityRef entityRef, bool toRemoveChildComponent)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	if (!entityRef.IsValid())
	{
		return;
	}
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (!registry.HaveComponents<ChildComponent>(entityRef.m_entity))
	{
		return;
	}
	registry.GetComponents<ChildrenComponent>(m_entity).RemoveChild(entityRef);
	if (toRemoveChildComponent) 
	{
		registry.RemoveComponents<ChildComponent>(entityRef.m_entity);
	}
}

void EntityRef::GetName(DString& outName) const
{
	DASSERT_E(IsValid());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<NameComponent>(m_entity));
	const NameComponent& nameComponent(registry.GetComponents<NameComponent>(m_entity));
	outName = nameComponent.GetName().Data();
}

void EntityRef::SetName(const DString& name)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<NameComponent>(m_entity));
	NameComponent& nameComponent(registry.GetComponents<NameComponent>(m_entity));
	nameComponent.SetName(name);
}

void EntityRef::GetUUID(UUIDType& outUUID) const
{
	DASSERT_E(IsValid());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<UUIDComponent>(m_entity));
	const UUIDComponent& uuidComponent(registry.GetComponents<UUIDComponent>(m_entity));
	outUUID = uuidComponent.GetUUID();
}

void EntityRef::Destroy()
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (registry.HaveComponents<ChildrenComponent>(m_entity))
	{
		ChildrenComponent& childrenComponent(registry.GetComponents<ChildrenComponent>(m_entity));
		std::vector<EntityRef> children(childrenComponent.GetChildren());
		for (EntityRef entityRef : children)
		{
			entityRef.Destroy();
		}
	}
	if (registry.HaveComponents<ChildComponent>(m_entity))
	{
		registry.GetComponents<ChildComponent>(m_entity).GetParent().RemoveChild(*this);
	}	
	registry.DestroyEntity
	(
		m_entity,
		[&](ComponentIdType componentId, void* component) -> void
		{
			const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
			componentForm.Destructor(component);
		}
	);
}

void EntityRef::RemoveParent()
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<ChildComponent>(m_entity));
	ChildComponent& childComponent(registry.GetComponents<ChildComponent>(m_entity));
	EntityRef parentRef(childComponent.GetParent());
	DASSERT_E(registry.HaveComponents<ChildrenComponent>(parentRef.m_entity));
	ChildrenComponent& parentChildrenComponent(registry.GetComponents<ChildrenComponent>(parentRef.m_entity));
	parentChildrenComponent.RemoveChild(*this);
	registry.RemoveComponents<ChildComponent>(m_entity);
	registry.AddComponents<RootComponent>(m_entity, std::make_tuple());
}

bool EntityRef::HaveChild(EntityRef entityRef) const
{
	DASSERT_E(IsValid());
	if (!entityRef.IsValid())
	{
		return false;
	}
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (!registry.HaveComponents<ChildrenComponent>(m_entity))
	{
		return false;
	}
	return registry.GetComponents<ChildrenComponent>(m_entity).HaveChild(entityRef);
}

bool EntityRef::HaveChildRecursive(EntityRef entityRef) const
{
	DASSERT_E(IsValid());
	if (!entityRef.IsValid())
	{
		return false;
	}
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	if (!registry.HaveComponents<ChildrenComponent>(m_entity)) 
	{
		return false;
	}
	const ChildrenComponent& childrenComponent(registry.GetComponents<ChildrenComponent>(m_entity));
	size_t numberOfChildren(childrenComponent.GetNumberOfChildren());
	EntityRef currentChild(childrenComponent.GetFirstChild());
	if (numberOfChildren == 0)
	{
		return false;
	}
	for (size_t i(0); i < numberOfChildren; i++)
	{
		if (currentChild == entityRef || currentChild.HaveChildRecursive(entityRef))
		{
			return true;
		}
		DASSERT_E(registry.HaveComponents<ChildComponent>(currentChild.m_entity));
		const ChildComponent& childComponent(registry.GetComponents<ChildComponent>(currentChild.m_entity));
		currentChild = childComponent.GetNext();
	}
	return false;
}

bool EntityRef::HaveChildren() const
{
	DASSERT_E(IsValid());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	return registry.HaveComponents<ChildrenComponent>(m_entity);
}

EntityRef::entityContainerType EntityRef::GetChildren() const
{
	DASSERT_E(IsValid() && HaveChildren());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	const ChildrenComponent& childrenComponent(registry.GetComponents<ChildrenComponent>(m_entity));
	return childrenComponent.GetChildren();
}

ComponentRef<Component> EntityRef::GetComponent(ComponentIdType componentId)
{
	DASSERT_E(IsValid());
	return ComponentRef<Component>(m_entity, m_internalSceneRef, componentId, *m_lockData);
}

const ComponentRef<Component> EntityRef::GetComponent(ComponentIdType componentId) const
{
	DASSERT_E(IsValid());
	return ComponentRef<Component>(m_entity, m_internalSceneRef, componentId, *m_lockData);
}

bool EntityRef::HaveComponent(DCore::ComponentIdType componentId) const
{
	DASSERT_E(IsValid());
	return m_internalSceneRef->GetAsset().GetRegistry().HaveComponents(m_entity, &componentId, 1);
}

bool EntityRef::HaveComponents(const ComponentIdType* componentIds, size_t numberOfComponents) const
{
	DASSERT_E(IsValid());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	return registry.HaveComponents(m_entity, componentIds, numberOfComponents);
}

void EntityRef::RemoveComponents(const ComponentIdType* componentIds, size_t numberOfComponents)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	m_internalSceneRef->GetAsset().GetRegistry().RemoveComponents
	(
		m_entity, componentIds, numberOfComponents,
		[&](ComponentIdType componentId, void* componentAddress) -> void
		{
			const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
			componentForm.Destructor(componentAddress);
		}
	);
}

void EntityRef::GetSceneUUID(UUIDType& outUUID) const
{
	DASSERT_E(IsValid());
	outUUID = m_internalSceneRef->GetUUID();
}

size_t EntityRef::GetNumberOfComponents() const
{
	DASSERT_E(IsValid());
	return m_internalSceneRef->GetAsset().GetRegistry().GetNumberOfComponents(m_entity);
}

void EntityRef::GetComponentIds(ComponentIdType* outComponentIds)
{
	DASSERT_E(IsValid());
	size_t index(0);
	return m_internalSceneRef->GetAsset().GetRegistry().IterateOnComponents
	(
		m_entity,
		[&](ComponentIdType componentId, void*) -> bool
		{
			outComponentIds[index++] = componentId;
			return false;
		}
	);
}

DMat4 EntityRef::GetWorldModelMatrix() const
{
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(IsValid() && registry.HaveComponents<TransformComponent>(m_entity));
	const TransformComponent& transform(registry.GetComponents<TransformComponent>(m_entity));
	DMat4 modelMatrix(transform.GetModelMatrix());
	IterateOnParents
	(
		[&](EntityRef entity) -> bool
		{
			if (!registry.HaveComponents<TransformComponent>(entity.m_entity))
			{
				return true;
			}
			const TransformComponent& parentTransform(registry.GetComponents<TransformComponent>(entity.m_entity));
			modelMatrix = parentTransform.GetModelMatrix() * modelMatrix; 
			return false;
		}
	);
	return modelMatrix;
}

DVec3 EntityRef::GetWorldTranslation() const
{
	DASSERT_E(IsValid());
	DMat4 modelMatrix(GetWorldModelMatrix());
	DVec3 translation;
	Math::GetTranslation(modelMatrix, translation);
	return translation;
}

void EntityRef::SetWorldTranslation(const DVec3& translation)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	DMat4 desiredModel(GetWorldModelMatrix());
	desiredModel[3][0] = translation.x;
	desiredModel[3][1] = translation.y;
	desiredModel[3][2] = translation.z;
	RemoveParentTransformationsFrom(desiredModel);
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<TransformComponent>(m_entity));
	TransformComponent& transform(registry.GetComponents<TransformComponent>(m_entity));
	transform.SetTranslation({desiredModel[3][0], desiredModel[3][1], desiredModel[3][2]});
}

DVec3 EntityRef::GetLocalTranslation() const
{
	DASSERT_E(IsValid());
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<TransformComponent>(m_entity));
	const TransformComponent& transform(registry.GetComponents<TransformComponent>(m_entity));
	return transform.GetTranslation();
}

void EntityRef::SetLocalTranslation(const DVec3& translation)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	DASSERT_E(registry.HaveComponents<TransformComponent>(m_entity));
	TransformComponent& transform(registry.GetComponents<TransformComponent>(m_entity));
	transform.SetTranslation(translation);
}

void EntityRef::RemoveParentTransformationsFrom(DMat4& model) const
{
	DASSERT_E(IsValid());
	DMat4 parentInverseTransformations(DMat4(1.0f));
	const Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
	IterateOnParents
	(
		[&](EntityRef entity) -> bool
		{
			DASSERT_E(registry.HaveComponents<TransformComponent>(entity.m_entity));
			const TransformComponent& parentTransform(registry.GetComponents<TransformComponent>(entity.m_entity));
			parentInverseTransformations *= parentTransform.GetInverseModelMatrix();
			return false;	
		}
	);
	model = parentInverseTransformations * model;
}

EntityRef EntityRef::Duplicate()
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	EntityRef parent;
	TryGetParent(parent);
	EntityRef duplicated(Duplicate(*this, parent));
	return duplicated;
}

EntityRef EntityRef::Duplicate(EntityRef entityToDuplicate, EntityRef parent)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *entityToDuplicate.m_lockData);
	DASSERT_E(entityToDuplicate.IsValid());
	Registry& registry(entityToDuplicate.m_internalSceneRef->GetAsset().GetRegistry());
	std::vector<ComponentIdType> componentIds;
	std::vector<size_t> componentSizes;
	registry.IterateOnComponents
	(
		entityToDuplicate.m_entity,
		[&](ComponentIdType componentId, void* component) -> bool
		{
			if (componentId == ComponentId::GetId<ChildComponent>() ||
				componentId == ComponentId::GetId<ChildrenComponent>())
			{
				return false;
			}
			const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
			componentIds.push_back(componentId);
			componentSizes.push_back(componentForm.TotalSize);
			return false;
		}
 	);
	EntityRef duplicated(
			registry.CreateEntity(
				componentIds.data(),
				componentSizes.data(),
				componentIds.size(),
				[&](ComponentIdType componentId, void* rawComponent) -> void
				{
					const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
					componentForm.PlacementNewConstructor(rawComponent, componentForm.DefaultArgs);
				}),
			entityToDuplicate.GetSceneRef());
	registry.IterateOnComponents
	(
		duplicated.m_entity,
		[&](ComponentIdType componentId, void* rawComponent) -> bool
		{
			const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
			ComponentRef<Component> toDuplicate(entityToDuplicate.GetComponent(componentId));
			Component* duplicatedComponent(static_cast<Component*>(rawComponent));
			for (const SerializedAttribute& attribute : componentForm.SerializedAttributes)
			{
				const AttributeIdType attributeId(attribute.GetAttributeId());
				void* attributePtr(toDuplicate.GetRawComponent()->GetAttributePtr(attributeId));
				const AttributeType attributeType(attribute.GetAttributeType());
				switch (attributeType)
				{
				case AttributeType::UUID:
				{
					UUIDType uuid;
					UUIDGenerator::Get().GenerateUUID(uuid);
					duplicatedComponent->OnAttributeChange(attributeId, &uuid, attributeType);
					break;
				}
				case AttributeType::SpriteMaterial:
					DuplicateAssetAndAssignItToComponent(
						attributeId,
						*static_cast<SpriteMaterialRef*>(attributePtr),
						attributeType,
						duplicatedComponent,
						[&](const UUIDType& uuid) -> SpriteMaterialRef {return AssetManager::Get().GetSpriteMaterial(uuid); });
					break;
				case AttributeType::Animation:
					DuplicateAssetAndAssignItToComponent(
						attributeId,
						*static_cast<AnimationRef*>(attributePtr),
						attributeType,
						duplicatedComponent,
						[&](const UUIDType& uuid) -> AnimationRef {return AssetManager::Get().GetAnimation(uuid); });
					break;
				case AttributeType::AnimationStateMachine:
				{
					AnimationStateMachineRef asmRef(*static_cast<AnimationStateMachineRef*>(attributePtr));
					if (!asmRef.IsValid())
					{
						break;
					}
					AnimationStateMachine duplicatedAsm(asmRef.GetInternalRef()->GetAsset());
					AnimationStateMachineRef duplicatedAsmRef(AssetManager::Get().AddAnimationStateMachine(std::move(duplicatedAsm)));
					duplicatedComponent->OnAttributeChange(attributeId, &duplicatedAsmRef, attributeType);
					break;
				}
				case AttributeType::PhysicsMaterial:
					DuplicateAssetAndAssignItToComponent(
						attributeId,
						*static_cast<PhysicsMaterialRef*>(attributePtr),
						attributeType,
						duplicatedComponent,
						[&](const UUIDType& uuid) -> PhysicsMaterialRef {return AssetManager::Get().GetPhysicsMaterial(uuid); });					
					break;
				default:
					duplicatedComponent->OnAttributeChange(attributeId, attributePtr, attributeType);
					break;
				}
			}
			return false;
		}
	);
	if (parent.IsValid())
	{
		duplicated.SetParent(parent);
	}
	if (entityToDuplicate.HaveChildren())
	{
		std::vector<EntityRef> children(entityToDuplicate.GetChildren());
		for (EntityRef childToDuplicate : children)
		{
			Duplicate(childToDuplicate, duplicated);
		}
	}
	return duplicated;
}

}
