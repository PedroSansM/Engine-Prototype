#pragma once

#include "DCoreAssert.h"
#include "ComponentPool.h"
#include "ECSTypes.h"
#include "SparseSet.h"
#include "ComponentId.h"
#include "TemplateUtils.h"
#include "ECSUtils.h"

#include <type_traits>
#include <vector>
#include <tuple>



namespace DCore
{

class Archetype
{
public:
	using componentIdsContainerType = SparseSet<ComponentIdType>;
	using entitySetType = SparseSet<Entity::idType>;
	using entityContainerType = std::vector<Entity>;
	using componentPoolContainerType = std::vector<ComponentPool>;
	using componentSizesContainerType = std::vector<size_t>;
public:
	Archetype(const ComponentIdType* componentIds, const size_t* componentSizes, size_t numberOfComponents)
	{
		for (size_t i(0); i < numberOfComponents; i++)
		{
			const ComponentIdType componentId(componentIds[i]);
			if (componentId == 0)
			{
				continue;
			}
			const size_t componentSize(componentSizes[i]);
			m_components.Add(componentId);
			m_componentPools.push_back(ComponentPool(componentSize));
			m_componentSizes.push_back(componentSize);
		}
	}

	Archetype(const Archetype& other)
		:
		m_components(other.m_components),
		m_componentPools(other.m_componentPools),
		m_entities(other.m_entities),
		m_entitySet(other.m_entitySet),
		m_componentSizes(other.m_componentSizes)
	{}

	Archetype(Archetype&& other) noexcept
		:
		m_components(std::move(other.m_components)),
		m_componentPools(std::move(other.m_componentPools)),
		m_entities(std::move(other.m_entities)),
		m_entitySet(std::move(other.m_entitySet)),
		m_componentSizes(std::move(other.m_componentSizes))
	{}

	~Archetype() = default;
public:
	bool HaveComponents(const ComponentIdType* componentIds, size_t numberOfComponents) const
	{
		if (numberOfComponents > m_components.Size())
		{
			return false;
		}
		for (size_t i(0); i < numberOfComponents; i++)
		{
			if (!m_components.Exists(componentIds[i]))
			{
				return false;
			}
		}
		return true;
	}

	bool HaveComponentsExactly(const ComponentIdType* componentIds, size_t numberOfComponents) const
	{
		size_t numberOfMatches(0);
		for (size_t i(0); i < numberOfComponents; i++)
		{
			const ComponentIdType componentId(componentIds[i]);
			if (componentId == 0)
			{
				continue;
			}
			if (!m_components.Exists(componentId))
			{
				return false;
			}
			numberOfMatches++;
		}
		return numberOfMatches == m_components.Size();
	}
	
	const ComponentIdType* GetComponentIds() const
	{
		return m_components.GetDenseRef().data();
	}

	size_t GetNumberOfComponents() const
	{
		return m_components.Size();
	}

	const size_t* GetComponentSizes() const
	{
		return m_componentSizes.data();
	}

	const size_t* GetComponentRefs() const
	{
		return m_components.GetSparseRef().data();
	}
public:
	template <class Func>
	void AddEntityComponents(Entity entity, const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(!m_entitySet.Exists(entity.GetIndex()));
		m_entities.push_back(entity);
		m_entitySet.Add(entity.GetIndex());
		for (size_t i(0); i < numberOfComponents; i++)
		{
			const ComponentIdType componentId(componentIds[i]);
			DASSERT_E(m_components.Exists(componentId));
			ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* component(componentPool.AddComponent());
			std::invoke(function, componentId, component);
		}
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	void AddEntityComponents(Entity entity, TupleArg&& tupleArg, TupleArgs&&... tupleArgs)
	{
		DASSERT_E(!m_entitySet.Exists(entity.GetIndex()));
		m_entities.push_back(entity);
		m_entitySet.Add(entity.GetIndex());
		AddEntityComponents<Component, Components...>(
			std::make_integer_sequence<size_t, std::tuple_size_v<TupleArg>>{}, 
			std::forward<TupleArg>(tupleArg), 
			std::forward<TupleArgs>(tupleArgs)...);
	}

	template <class Func>
	void GetComponents(const ComponentIdType* componentIds, size_t numberOfComponents, size_t entityIndex, Func function)
	{
		DASSERT_E(m_entitySet.Exists(entityIndex));
		const size_t entityComponentIndex(m_entitySet.GetIndexTo(entityIndex));
		for (size_t i(0); i < numberOfComponents; i++)
		{
			const ComponentIdType componentId(componentIds[i]);
			DASSERT_E(m_components.Exists(componentId));
			ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* component(componentPool.GetComponentAtIndex(entityComponentIndex));
			std::invoke(function, componentId, component);
		}
	}

	template <class Func>
	void GetComponents(const ComponentIdType* componentIds, size_t numberOfComponents, size_t entityIndex, Func function) const
	{
		DASSERT_E(m_entitySet.Exists(entityIndex));
		const size_t entityComponentIndex(m_entitySet.GetIndexTo(entityIndex));
		for (size_t i(0); i < numberOfComponents; i++)
		{
			const ComponentIdType componentId(componentIds[i]);
			DASSERT_E(m_components.Exists(componentId));
			const ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			const void* component(componentPool.GetComponentAtIndex(entityComponentIndex));
			std::invoke(function, componentId, component);
		}
	}

	template <class Component, class ...Components>
	decltype(auto) GetComponents(size_t entityIndex)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_entitySet.Exists(entityIndex) && m_components.Exists(componentId));
		const size_t componentPoolIndex(m_components.GetIndexTo(componentId));
		Component* component(static_cast<Component*>(m_componentPools[componentPoolIndex].GetComponentAtIndex(m_entitySet.GetIndexTo(entityIndex))));
		if constexpr (TypeList<Components...>::size == 0)
		{
			return *component;
		}
		else
		{
			return GetComponents(entityIndex, std::make_tuple(component), TypeList<Components...>{});
		}
	}

	template <class Component, class ...Components>
	decltype(auto) GetComponents(size_t entityIndex) const
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_entitySet.Exists(entityIndex) && m_components.Exists(componentId));
		const size_t componentPoolIndex(m_components.GetIndexTo(componentId));
		const Component* component(static_cast<const Component*>(m_componentPools[componentPoolIndex].GetComponentAtIndex(m_entitySet.GetIndexTo(entityIndex))));
		if constexpr (TypeList<Components...>::size == 0)
		{
			return *component;
		}
		else
		{
			return GetComponents(entityIndex, std::make_tuple(component), TypeList<Components...>{});
		}
	}
	template <class Func>
	bool Iterate(const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		for (size_t i(0); i < m_entities.size(); i++)
		{
			Entity entity(m_entities[i]);
			size_t entityComponentIndex(m_entitySet.GetIndexTo(entity.GetIndex()));
			for (size_t j(0); j < numberOfComponents; j++)
			{
				const ComponentIdType componentId(componentIds[j]);
				DASSERT_E(m_components.Exists(componentId));
				ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
				void* component(componentPool.GetComponentAtIndex(entityComponentIndex));
				if (std::invoke(function, entity, componentId, component))
				{
					return true;
				}
			}
		}
		return false;
	}

	template <class Component, class ...Components, class Func>
	bool Iterate(Func function)
	{
		const size_t numberOfEntities(m_entities.size());
		for (size_t i(0); i < numberOfEntities; i++)
		{
			Entity entity(m_entities[i]);
			const ComponentIdType componentId(ComponentId::GetId<Component>());
			DASSERT_E(m_components.Exists(componentId));
			ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			size_t entityComponentIndex(m_entitySet.GetIndexTo(entity.GetIndex()));
			Component* component(static_cast<Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));	
			if (Iterate(entity, entityComponentIndex, function, TypeList<Components...>{}, *component))
			{
				return true;
			}
		}
		return false;
	}

	template <class Component, class ...Components, class Func>
	bool Iterate(Func function) const
	{
		const size_t numberOfEntities(m_entities.size());
		for (size_t i(0); i < numberOfEntities; i++)
		{
			Entity entity(m_entities[i]);
			const ComponentIdType componentId(ComponentId::GetId<Component>());
			DASSERT_E(m_components.Exists(componentId));
			const ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			size_t entityComponentIndex(m_entitySet.GetIndexTo(entity.GetIndex()));
			const Component* component(static_cast<const Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));	
			if (Iterate(entity, entityComponentIndex, function, TypeList<Components...>{}, *component))
			{
				return true;
			}
		}
		return false;
	}

	template <class Func>
	void IterateOnComponents(Entity entity, Func function)
	{
		DASSERT_E(m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(m_entitySet.GetIndexTo(entity.GetIndex()));
		for (ComponentIdType componentId : m_components)
		{
			ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* component(componentPool.GetComponentAtIndex(entityComponentIndex));
			if (std::invoke(function, componentId, component))
			{
				return;
			}
		}
	}

	template <class Func>
	void AddEntityFromArchetypeWithLessComponents(Entity entity, Archetype& other, const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(!m_entitySet.Exists(entity.GetIndex()) && other.m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(other.m_entitySet.GetIndexTo(entity.GetIndex()));
		for (const ComponentIdType componentId : other.m_components.GetDenseRef())
		{
			DASSERT_E(m_components.Exists(componentId));
			ComponentPool& otherComponentPool(other.m_componentPools[other.m_components.GetIndexTo(componentId)]);
			void* otherComponentAddress(otherComponentPool.GetComponentAtIndex(entityComponentIndex));
			ComponentPool& targetComponentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* newComponentAddress(targetComponentPool.AddComponent());
			std::memcpy(newComponentAddress, otherComponentAddress, targetComponentPool.GetChunkSize());
			otherComponentPool.RemoveComponent(entityComponentIndex);
		}
		other.m_entities.erase(other.m_entities.begin() + entityComponentIndex);
		other.m_entitySet.KeepOrderRemove(entity.GetIndex());
		AddEntityComponents(entity, componentIds, numberOfComponents, function);
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	void AddEntityFromArchetypeWithLessComponents(Entity entity, Archetype& other, TupleArg&& arg, TupleArgs&&... args)
	{
		DASSERT_E(!m_entitySet.Exists(entity.GetIndex()) && other.m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(other.m_entitySet.GetIndexTo(entity.GetIndex()));
		m_entities.push_back(entity);
		m_entitySet.Add(entity.GetIndex());
		for (const ComponentIdType componentId : other.m_components.GetDenseRef())
		{
			DASSERT_E(m_components.Exists(componentId));
			ComponentPool& otherComponentPool(other.m_componentPools[other.m_components.GetIndexTo(componentId)]);
			void* otherComponentAddress(otherComponentPool.GetComponentAtIndex(entityComponentIndex));
			ComponentPool& targetComponentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* newComponentAddress(targetComponentPool.AddComponent());
			std::memcpy(newComponentAddress, otherComponentAddress, targetComponentPool.GetChunkSize());
			otherComponentPool.RemoveComponent(entityComponentIndex);
		}
		other.m_entities.erase(other.m_entities.begin() + entityComponentIndex);
		other.m_entitySet.KeepOrderRemove(entity.GetIndex());
		AddEntityComponents<Component, Components...>(
			std::make_integer_sequence<size_t, std::tuple_size_v<TupleArg>>{}, 
			std::forward<TupleArg>(arg), 
			std::forward<TupleArgs>(args)...);
	}

	template <class Func>
	void DestroyEntity(Entity entity, Func function)
	{
		DASSERT_E(m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(m_entitySet.GetIndexTo(entity.GetIndex()));
		for (DCore::ComponentIdType componentId : m_components)
		{
			ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* component(componentPool.GetComponentAtIndex(entityComponentIndex));
			std::invoke(function, componentId, component);
			componentPool.RemoveComponent(entityComponentIndex);
		}
		m_entities.erase(m_entities.begin() + entityComponentIndex);
		m_entitySet.KeepOrderRemove(entity.GetIndex());
	}

	template <class Component, class ...Components>
	void DestroyEntity(Entity entity)
	{
		DASSERT_E(m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(m_entitySet.GetIndexTo(entity.GetIndex()));
		constexpr size_t numberOfComponentsToRemove(TypeList<Component, Components...>::size);
		DASSERT_E(numberOfComponentsToRemove == m_components.Size());
		m_entities.erase(m_entities.begin() + entityComponentIndex);
		m_entitySet.KeepOrderRemove(entity.GetIndex());
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
		Component* component(static_cast<Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));
		component->~Component();
		DestroyEntity(entityComponentIndex, TypeList<Components...>{});
	}

	template <class Func>
	void AddEntityFromArchetypeWithMoreComponents(Entity entity, Archetype& other, Func function)
	{
		DASSERT_E(!m_entitySet.Exists(entity.GetIndex()) && other.m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(other.m_entitySet.GetIndexTo(entity.GetIndex()));
		for (size_t i(0); i < other.m_components.Size(); i++)
		{
			const ComponentIdType componentId(other.m_components.GetDenseRef()[i]);
			ComponentPool& otherComponentPool(other.m_componentPools[other.m_components.GetIndexTo(componentId)]);
			if (!m_components.Exists(componentId))
			{
				void* component(otherComponentPool.GetComponentAtIndex(entityComponentIndex));
				std::invoke(function, componentId, component);
				otherComponentPool.RemoveComponent(entityComponentIndex);
				continue;
			}
			ComponentPool& thisComponentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* otherComponentAddress(otherComponentPool.GetComponentAtIndex(entityComponentIndex));
			void* newComponentAddress(thisComponentPool.AddComponent());
			std::memcpy(newComponentAddress, otherComponentAddress, other.GetComponentSizes()[i]);
			otherComponentPool.RemoveComponent(entityComponentIndex);
		}
		other.m_entities.erase(other.m_entities.begin() + entityComponentIndex);
		other.m_entitySet.KeepOrderRemove(entity.GetIndex());
		m_entities.push_back(entity);
		m_entitySet.Add(entity.GetIndex());
	}

	template <class Component, class ...Components>
	void AddEntityFromArchetypeWithMoreComponents(Entity entity, Archetype& other)
	{
		DASSERT_E(!m_entitySet.Exists(entity.GetIndex()) && other.m_entitySet.Exists(entity.GetIndex()));
		const size_t entityComponentIndex(other.m_entitySet.GetIndexTo(entity.GetIndex()));
		other.m_entities.erase(other.m_entities.begin() + entityComponentIndex);
		other.m_entitySet.KeepOrderRemove(entity.GetIndex());
		m_entities.push_back(entity);
		m_entitySet.Add(entity.GetIndex());
		for (size_t i(0); i < m_components.Size(); i++)
		{
			const ComponentIdType componentId(m_components.GetDenseRef()[i]);
			DASSERT_E(other.m_components.Exists(componentId));
			ComponentPool& otherComponentPool(other.m_componentPools[other.m_components.GetIndexTo(componentId)]);
			void* otherComponentAddress(otherComponentPool.GetComponentAtIndex(entityComponentIndex));
			ComponentPool& thisComponentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
			void* newComponentAddress(thisComponentPool.AddComponent());
			std::memcpy(newComponentAddress, otherComponentAddress, GetComponentSizes()[i]);
			otherComponentPool.RemoveComponent(entityComponentIndex);
		}
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(!m_components.Exists(componentId) && other.m_components.Exists(componentId));
		ComponentPool& otherComponentPool(other.m_componentPools[other.m_components.GetIndexTo(componentId)]);
		Component* component(static_cast<Component*>(otherComponentPool.GetComponentAtIndex(entityComponentIndex)));
		component->~Component();
		otherComponentPool.RemoveComponent(entityComponentIndex);
		AddEntityFromArchetypeWithMoreComponents(entityComponentIndex, other, TypeList<Components...>{});
	}
private:
	componentIdsContainerType m_components;
	componentPoolContainerType m_componentPools;
	entityContainerType m_entities;
	entitySetType m_entitySet;
	componentSizesContainerType m_componentSizes;
private:
	void AddEntityComponents(TypeList<>)
	{}

	void DestroyEntity(size_t entityComponentIndex, TypeList<>)
	{}

	void AddEntityFromArchetypeWithMoreComponents(size_t entityComponentIndex, Archetype& other, TypeList<>)
	{}
private:
	template <class Component, class ...Components, class TupleArg, class ...TupleArgs, size_t ...Ints>
	void AddEntityComponents(std::integer_sequence<size_t, Ints...>, TupleArg&& tupleArg, TupleArgs&&... tupleArgs)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
		void* component(componentPool.AddComponent());
		if constexpr (std::tuple_size_v<TupleArg> == 0)
		{
			new (component) Component();
		}
		else
		{
			if constexpr (std::is_constructible_v<Component, decltype(std::get<Ints>(tupleArg))...>)
			{
				new (component) Component(std::get<Ints>(std::forward<TupleArg>(tupleArg))...);
			}
			else
			{
				new (component) Component{std::get<Ints>(std::forward<TupleArg>(tupleArg))...};
			}
		}
		AddEntityComponents(TypeList<Components...>{}, std::forward<TupleArgs>(tupleArgs)...);
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	void AddEntityComponents(TypeList<Component, Components...>, TupleArg&& tupleArg, TupleArgs&&... tupleArgs)
	{
		AddEntityComponents<Component, Components...>(
			std::make_integer_sequence<size_t, std::tuple_size_v<TupleArg>>{}, 
			std::forward<TupleArg>(tupleArg), 
			std::forward<TupleArgs>(tupleArgs)...);
	}

	template <class TupleType, class Component, class ...Components>
	decltype(auto) GetComponents(size_t entityIndex, TupleType componentTuple, TypeList<Component, Components...>)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		const size_t componentPoolIndex(m_components.GetIndexTo(componentId));
		Component* component(static_cast<Component*>(m_componentPools[componentPoolIndex].GetComponentAtIndex(m_entitySet.GetIndexTo(entityIndex))));
		return GetComponents(entityIndex, std::tuple_cat(componentTuple, std::make_tuple(component)), TypeList<Components...>{});
	}

	template <class TupleType, class Component, class ...Components>
	decltype(auto) GetComponents(size_t entityIndex, TupleType componentTuple, TypeList<Component, Components...>) const
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		const size_t componentPoolIndex(m_components.GetIndexTo(componentId));
		const Component* component(static_cast<const Component*>(m_componentPools[componentPoolIndex].GetComponentAtIndex(m_entitySet.GetIndexTo(entityIndex))));
		return GetComponents(entityIndex, std::tuple_cat(componentTuple, std::make_tuple(component)), TypeList<Components...>{});
	}

	template <class TupleType>
	TupleType GetComponents(size_t entityIndex, TupleType componentTuple, TypeList<>)
	{
		return componentTuple;
	}

	template <class TupleType>
	TupleType GetComponents(size_t entityIndex, TupleType componentTuple, TypeList<>) const
	{
		return componentTuple;
	}

	template <class Component, class ...Components, class Func, class ...Args>
	bool Iterate(Entity entity, size_t entityComponentIndex, Func function, TypeList<Component, Components...>, Args&... args)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
		Component* component(static_cast<Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));	
		return Iterate(entity, entityComponentIndex, function, TypeList<Components...>{}, args..., *component);	
	}

	template <class Component, class ...Components, class Func, class ...Args>
	bool Iterate(Entity entity, size_t entityComponentIndex, Func function, TypeList<Component, Components...>, const Args&... args) const
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		const ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
		const Component* component(static_cast<const Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));	
		return Iterate(entity, entityComponentIndex, function, TypeList<Components...>{}, args..., *component);	
	}

	template <class Func, class ...Args>
	bool Iterate(Entity entity, size_t entityComponentIndex, Func function, TypeList<>, Args&... args)
	{
		return std::invoke(function, entity, args...);
	}

	template <class Func, class ...Args>
	bool Iterate(Entity entity, size_t entityComponentIndex, Func function, TypeList<>, const Args&... args) const
	{
		return std::invoke(function, entity, args...);
	}

	template <class Component, class ...Components>
	void DestroyEntity(size_t entityComponentIndex, TypeList<Component, Components...>)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(m_components.Exists(componentId));
		ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
		Component* component(static_cast<Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));
		component->~Component();
		DestroyEntity(entityComponentIndex, TypeList<Components...>{});
	}

	template <class Component, class ...Components>
	void AddEntityFromArchetypeWithMoreComponents(size_t entityComponentIndex, Archetype& other, TypeList<Component, Components...>)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		DASSERT_E(!m_components.Exists(componentId) && other.m_components.Exists(componentId));
		ComponentPool& componentPool(m_componentPools[m_components.GetIndexTo(componentId)]);
		Component* component(static_cast<Component*>(componentPool.GetComponentAtIndex(entityComponentIndex)));
		component->~Component();
		AddEntityFromArchetypeWithMoreComponents(entityComponentIndex, other, TypeList<Components...>{});
	}
};

}
