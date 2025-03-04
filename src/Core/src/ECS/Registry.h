#pragma once

#include "ReciclingVector.h"
#include "EntityInfo.h"
#include "ECSTypes.h"
#include "Archetype.h"
#include "ECSUtils.h"
#include "TemplateUtils.h"

#include <tuple>
#include <utility>



namespace DCore
{

class Registry
{
public:
	using entityContainerType = ReciclingVector<EntityInfo, EntityIdType, EntityVersionType>;
	using archetypeContainerType = ReciclingVector<Archetype>;
public:
	Registry() = default;

	Registry(const Registry& other)
		:
		m_entities(other.m_entities),
		m_archetypes(other.m_archetypes)
	{}

	Registry(Registry&& other) noexcept
		:
		m_entities(std::move(other.m_entities)),
		m_archetypes(std::move(other.m_archetypes))
	{}

	~Registry() = default;
public:
	bool HaveComponents(Entity entity, const ComponentIdType* componentIds, size_t numberOfComponents) const
	{
		DASSERT_E(entity.IsValid());
		const Archetype& archetype(m_archetypes[m_entities[entity.GetIndex()].ArchetypeIndex]);
		return archetype.HaveComponents(componentIds, numberOfComponents);
	}

	size_t GetNumberOfComponents(Entity entity) const
	{
		DASSERT_E(entity.IsValid());
		const Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		return archetype.GetNumberOfComponents();
	}

	Entity GetEntityWithIdAndVersion(EntityIdType id, EntityVersionType version) const
	{
		DASSERT_E(id != 0);
		Entity entity(m_entities.GetRefFromIndex(id - 1));
		if (entity.GetVersion() != version)
		{
			entity.Invalidate();
		}
		return entity;
	}
public:
	template <class Func>
	Entity CreateEntity(const ComponentIdType* componentIds, const size_t* componentSizes, size_t numberOfComponents, Func function)
	{
		entityContainerType::Ref entity;
		archetypeContainerType::Ref archetype;
		if (TryGetArchetypeWithComponentsExactly(componentIds, numberOfComponents, archetype))
		{
			entity = m_entities.PushBack(archetype.GetIndex());
			archetype->AddEntityComponents(entity, componentIds, numberOfComponents, function);
		}
		else
		{
			entity = m_entities.PushBack(size_t(0));
			archetype = m_archetypes.PushBack(componentIds, componentSizes, numberOfComponents);
			archetype->AddEntityComponents(entity, componentIds, numberOfComponents, function);
			entity->ArchetypeIndex = archetype.GetIndex();
		}
		return entityContainerType::ConstRef(entity.GetId(), entity.GetVersion(), *entity.GetReciclingVector());
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	Entity CreateEntity(TupleArg&& tupleArg, TupleArgs&&... tupleArgs)
	{
		entityContainerType::Ref entity;
		archetypeContainerType::Ref archetype;
		using typesToIdsType = ComponentTypesToComponentIds<Component, Components...>;
		typesToIdsType typesToIds;
		ComponentIdType* componentIds(typesToIds.Get());
		if (TryGetArchetypeWithComponentsExactly(typesToIds.Get(), typesToIdsType::numberOfComponents, archetype))
		{
			entity = m_entities.PushBack(archetype.GetIndex());
			archetype->AddEntityComponents<Component, Components...>(entity, std::forward<TupleArg>(tupleArg), std::forward<TupleArgs>(tupleArgs)...);
		}
		else
		{
			using typesToSizesType = ComponentTypesToComponentSizes<Component, Components...>;
			typesToSizesType typeToSizes;
			archetype = m_archetypes.PushBack(componentIds, typeToSizes.Get(), typesToIdsType::numberOfComponents);
			entity = m_entities.PushBack(size_t(0));
			archetype->AddEntityComponents<Component, Components...>(entity, std::forward<TupleArg>(tupleArg), std::forward<TupleArgs>(tupleArgs)...);
			entity->ArchetypeIndex = archetype.GetIndex();
		}
		return entityContainerType::ConstRef(entity.GetId(), entity.GetVersion(), *entity.GetReciclingVector());
	}

	template <class Func>
	void DestroyEntity(Entity entity, Func function)
	{
		DASSERT_E(entity.IsValid());
		Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		archetype.DestroyEntity(entity, function);
		m_entities.Remove(entity);
	}

	template <class Func>
	void GetComponents(Entity entity, const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(entity.IsValid());
		Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		archetype.GetComponents(componentIds, numberOfComponents, entity.GetIndex(), function);
	}

	template <class Func>
	void GetComponents(Entity entity, const ComponentIdType* componentIds, size_t numberOfComponents, Func function) const
	{
		DASSERT_E(entity.IsValid());
		const Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		archetype.GetComponents(componentIds, numberOfComponents, entity.GetIndex(), function);
	}
	
	template <class Component, class ...Components>
	decltype(auto) GetComponents(Entity entity)
	{
		DASSERT_E(entity.IsValid());
		Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		return archetype.GetComponents<Component, Components...>(entity.GetIndex());
	}

	template <class Component, class ...Components>
	decltype(auto) GetComponents(Entity entity) const
	{
		DASSERT_E(entity.IsValid());
		const Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		return archetype.GetComponents<Component, Components...>(entity.GetIndex());
	}

	template <class Func>
	void Iterate(const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponents(componentIds, numberOfComponents))
				{
					if (archetype->Iterate(componentIds, numberOfComponents, function))
					{
						return true;
					}
				}
				return false;
			}
		);
	}

	template <class Component, class ...Components, class Func>
	void Iterate(Func function)
	{
		using TypesToIds = ComponentTypesToComponentIds<Component, Components...>;
		TypesToIds typesToIds;
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponents(typesToIds.Get(), TypesToIds::numberOfComponents))
				{
					if (archetype->Iterate<Component, Components...>(function))
					{
						return true;
					}
				}
				return false;
			}
		);
	}

	template <class Component, class ...Components, class Func>
	void Iterate(Func function) const
	{
		using TypesToIds = ComponentTypesToComponentIds<Component, Components...>;
		TypesToIds typesToIds;
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::ConstRef archetype) -> bool
			{
				if (archetype->HaveComponents(typesToIds.Get(), TypesToIds::numberOfComponents))
				{
					if (archetype->Iterate<Component, Components...>(function))
					{
						return true;
					}
				}
				return false;
			}
		);
	}

	template <class Func>
	void IterateOnComponents(Entity entity, Func function)
	{
		DASSERT_E(entity.IsValid());
		Archetype& archetype(m_archetypes[entity->ArchetypeIndex]);
		archetype.IterateOnComponents(entity, function);
	}

	template <class Func>
	void IterateOnEntities(Func function)
	{
		m_entities.Iterate
		(
			[&](Entity entity) -> bool
			{
				return std::invoke(function, entity);
			}
		);
	}

	template <class Func>
	void IterateOnEntities(Func function) const
	{
		m_entities.Iterate
		(
			[&](Entity entity) -> bool
			{
				return std::invoke(function, entity);
			}
		);
	}

	template <class Func>
	void AddComponents(Entity entity, const ComponentIdType* componentIds, const size_t* componentSizes, size_t numberOfComponents, Func function)
	{
		DASSERT_E(entity.IsValid());
		archetypeContainerType::Ref fromArchetype(m_archetypes.GetRefFromIndex(m_entities[entity.GetIndex()].ArchetypeIndex));
		const size_t fromArchetypeNumberOfComponents(fromArchetype->GetNumberOfComponents());
		const size_t totalNumberOfComponents(fromArchetypeNumberOfComponents + numberOfComponents);
		ComponentIdType* allComponentIds(static_cast<ComponentIdType*>(alloca(totalNumberOfComponents * sizeof(ComponentIdType))));
		std::memcpy(allComponentIds, fromArchetype->GetComponentIds(), fromArchetypeNumberOfComponents * sizeof(ComponentIdType));
		std::memcpy(allComponentIds + fromArchetypeNumberOfComponents, componentIds, numberOfComponents * sizeof(ComponentIdType));
		bool movementDone(false);
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponentsExactly(allComponentIds, totalNumberOfComponents))
				{
					archetype->AddEntityFromArchetypeWithLessComponents(entity, *fromArchetype.Data(), componentIds, numberOfComponents, function);
					m_entities[entity.GetIndex()].ArchetypeIndex = archetype.GetIndex();
					movementDone = true;
					return true;
				}
				return false;
			}
		);
		if (movementDone)
		{
			return;
		}
		size_t* allComponentSizes(static_cast<size_t*>(alloca(totalNumberOfComponents * sizeof(size_t))));
		std::memcpy(allComponentSizes, fromArchetype->GetComponentSizes(), fromArchetypeNumberOfComponents * sizeof(size_t));
		std::memcpy(allComponentSizes + fromArchetypeNumberOfComponents, componentSizes, numberOfComponents * sizeof(size_t));
		archetypeContainerType::Ref newArchetype(m_archetypes.PushBack(allComponentIds, allComponentSizes, totalNumberOfComponents));
		newArchetype->AddEntityFromArchetypeWithLessComponents(entity, *fromArchetype.Data(), componentIds, numberOfComponents, function);
		m_entities[entity.GetIndex()].ArchetypeIndex = newArchetype.GetIndex();
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	void AddComponents(Entity entity, TupleArg&& tupleArg, TupleArgs&&... tupleArgs)
	{
		DASSERT_E(entity.IsValid());
		archetypeContainerType::Ref fromArchetype(m_archetypes.GetRefFromIndex(m_entities[entity.GetIndex()].ArchetypeIndex));
		const size_t fromArchetypeNumberOfComponents(fromArchetype->GetNumberOfComponents());
		constexpr size_t numberOfComponentsToAdd(TypeList<Component, Components...>::size);
		const size_t totalNumberOfComponents(fromArchetypeNumberOfComponents + numberOfComponentsToAdd);
		ComponentIdType* allComponentIds(static_cast<ComponentIdType*>(alloca(totalNumberOfComponents * sizeof(ComponentIdType))));
		std::memcpy(allComponentIds, fromArchetype->GetComponentIds(), fromArchetypeNumberOfComponents * sizeof(ComponentIdType));
		using typesToIdsType = ComponentTypesToComponentIds<Component, Components...>;
		typesToIdsType typeToIds;
		std::memcpy(allComponentIds + fromArchetypeNumberOfComponents, typeToIds.Get(), numberOfComponentsToAdd * sizeof(ComponentIdType));
		bool movementDone(false);
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponentsExactly(allComponentIds, totalNumberOfComponents))
				{
					archetype->AddEntityFromArchetypeWithLessComponents<Component, Components...>(entity, *fromArchetype.Data(), std::forward<TupleArg>(tupleArg), std::forward<TupleArgs>(tupleArgs)...);
					m_entities[entity.GetIndex()].ArchetypeIndex = archetype.GetIndex();
					movementDone = true;
					return true;
				}
				return false;
			}
		);
		if (movementDone)
		{
			return;
		}
		size_t* allComponentSizes(static_cast<size_t*>(alloca(totalNumberOfComponents * sizeof(size_t))));
		std::memcpy(allComponentSizes, fromArchetype->GetComponentSizes(), fromArchetypeNumberOfComponents * sizeof(size_t));
		using typeToSizesType = ComponentTypesToComponentSizes<Component, Components...>;
		typeToSizesType typesToSizes;
		std::memcpy(allComponentSizes + fromArchetypeNumberOfComponents, typesToSizes.Get(),  numberOfComponentsToAdd * sizeof(size_t));
		archetypeContainerType::Ref newArchetype(m_archetypes.PushBack(allComponentIds, allComponentSizes, totalNumberOfComponents));
		newArchetype->AddEntityFromArchetypeWithLessComponents<Component, Components...>(entity, *fromArchetype.Data(), std::forward<TupleArg>(tupleArg), std::forward<TupleArgs>(tupleArgs)...);
		m_entities[entity.GetIndex()].ArchetypeIndex = newArchetype.GetIndex();
	}

	template <class Func>
	void RemoveComponents(Entity entity, const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(entity.IsValid());
		archetypeContainerType::Ref fromArchetype(m_archetypes.GetRefFromIndex(m_entities[entity.GetIndex()].ArchetypeIndex));
		const size_t fromArchetypeNumberOfComponents(fromArchetype->GetNumberOfComponents());
		DASSERT_E(fromArchetypeNumberOfComponents >= numberOfComponents);
		if (fromArchetypeNumberOfComponents == numberOfComponents)
		{
			fromArchetype->DestroyEntity(entity, function);
			m_entities.Remove(entity);
			return;
		}
		ComponentIdType* leftComponentIds(static_cast<ComponentIdType*>(alloca(fromArchetypeNumberOfComponents * sizeof(ComponentIdType))));
		std::memcpy(leftComponentIds, fromArchetype->GetComponentIds(), fromArchetypeNumberOfComponents * sizeof(ComponentIdType));
		const size_t* leftComponentRefs(fromArchetype->GetComponentRefs());
		for (size_t i(0); i < numberOfComponents; i++)
		{
			leftComponentIds[leftComponentRefs[componentIds[i]]] = 0;
		}
		bool movementDone(false);
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponentsExactly(leftComponentIds, fromArchetypeNumberOfComponents))
				{
					movementDone = true;
					archetype->AddEntityFromArchetypeWithMoreComponents(entity, *fromArchetype.Data(), function);
					m_entities[entity.GetIndex()].ArchetypeIndex = archetype.GetIndex();
					return true;
				}
				return false;
			}
		);
		if (movementDone)
		{
			return;
		}
		archetypeContainerType::Ref newArchetype(m_archetypes.PushBack(leftComponentIds, fromArchetype->GetComponentSizes(), fromArchetypeNumberOfComponents));
		newArchetype->AddEntityFromArchetypeWithMoreComponents(entity, *fromArchetype.Data(), function);
		m_entities[entity.GetIndex()].ArchetypeIndex = newArchetype.GetIndex();
	}	

	template <class Component, class ...Components>
	void RemoveComponents(Entity entity)
	{
		DASSERT_E(entity.IsValid());
		constexpr size_t numberOfComponentsToRemove(TypeList<Component, Components...>::size);
		archetypeContainerType::Ref fromArchetype(m_archetypes.GetRefFromIndex(m_entities[entity.GetIndex()].ArchetypeIndex));
		const size_t fromArchetypeNumberOfComponents(fromArchetype->GetNumberOfComponents());
		DASSERT_E(fromArchetypeNumberOfComponents >= numberOfComponentsToRemove);
		if (fromArchetypeNumberOfComponents == numberOfComponentsToRemove)
		{
			fromArchetype->DestroyEntity<Component, Components...>(entity);
			m_entities.Remove(entity);
			return;
		}
		ComponentIdType* leftComponentIds(static_cast<ComponentIdType*>(alloca(fromArchetypeNumberOfComponents * sizeof(ComponentIdType))));
		std::memcpy(leftComponentIds, fromArchetype->GetComponentIds(), fromArchetypeNumberOfComponents * sizeof(ComponentIdType));
		const size_t* leftComponentRefs(fromArchetype->GetComponentRefs());
		using typesToIdsType = ComponentTypesToComponentIds<Component, Components...>;
		typesToIdsType typesToIds;
		for (size_t i(0); i < typesToIdsType::numberOfComponents; i++)
		{
			leftComponentIds[leftComponentRefs[typesToIds.Get()[i]]] = 0;
		}
		bool movementDone(false);
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponentsExactly(leftComponentIds, fromArchetypeNumberOfComponents))
				{
					movementDone = true;
					archetype->AddEntityFromArchetypeWithMoreComponents<Component, Components...>(entity, *fromArchetype.Data());
					m_entities[entity.GetIndex()].ArchetypeIndex = archetype.GetIndex();
					return true;
				}
				return false;
			}
		);
		if (movementDone)
		{
			return;
		}
		archetypeContainerType::Ref newArchetype(m_archetypes.PushBack(leftComponentIds, fromArchetype->GetComponentSizes(), fromArchetypeNumberOfComponents));
		newArchetype->AddEntityFromArchetypeWithMoreComponents<Component, Components...>(entity, *fromArchetype.Data());
		m_entities[entity.GetIndex()].ArchetypeIndex = newArchetype.GetIndex();
	}

	template <class Component, class ...Components>
	bool HaveComponents(Entity entity) const
	{
		DASSERT_E(entity.IsValid());
		const Archetype& archetype(m_archetypes[m_entities[entity.GetIndex()].ArchetypeIndex]);
		using typesToIdsType = ComponentTypesToComponentIds<Component, Components...>;
		typesToIdsType typesToIds;
		return archetype.HaveComponents(typesToIds.Get(), typesToIdsType::numberOfComponents);
	}
private:
	entityContainerType m_entities;	
	archetypeContainerType m_archetypes;
private:
	bool TryGetArchetypeWithComponentsExactly(const ComponentIdType* componentIds, size_t numberOfComponents, archetypeContainerType::Ref& out)
	{
		bool returnValue(false);
		m_archetypes.Iterate
		(
			[&](archetypeContainerType::Ref archetype) -> bool
			{
				if (archetype->HaveComponentsExactly(componentIds, numberOfComponents))
				{
					out = archetype;
					returnValue = true;
					return true;
				}
				return false;
			}
		);
		return returnValue;
	}
};

}
