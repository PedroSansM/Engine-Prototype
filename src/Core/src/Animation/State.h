#pragma once

#include "Animation.h"
#include "Transition.h"
#include "SparseSet.h"
#include "EntityRef.h"
#include "AnimationSimulator.h"

#include <vector>



namespace DCore
{

class State
{
	friend class AnimationStateMachine;
public:
	using stringType = std::string;
	using toStateIndexesType = SparseSet<>;
public:
	State(const stringType& name);
	State(
		const State&,
		integerParameterContainerType&,
		floatParameterContainerType&,
		logicParameterContainerType&,
		triggerParameterContainerType&);
	State(
		State&&,
		integerParameterContainerType&,
		floatParameterContainerType&,
		logicParameterContainerType&,
		triggerParameterContainerType&) noexcept;
	State(State&&) noexcept;
	~State();
public:
	void AddTransition(size_t fromStateIndex, size_t toStateIndex, transitionConstRefType* outTransition = nullptr);
	transitionConstRefType TryGetTransitionAtIndex(size_t index) const;
	void DeleteTransitionAtIndex(size_t transitionIndex);
	void DeleteAllTransitionsThatGoToStateAtIndex(size_t);
	bool TryMakeTransition(size_t& outToStateIndex);
public:
	const stringType& GetName() const
	{
		return m_name;
	}

	void SetName(const stringType& name)
	{
		m_name = name;
	}

	void SetAnimation(AnimationRef animation)
	{
		m_animation = animation;
	}

	const AnimationRef GetAnimation() const
	{
		return m_animation;
	}

	bool TransitionExists(size_t toStateIndex) const
	{
		return m_toStateIndexes.Exists(toStateIndex);
	}
	
	transitionRefType TryGetTransitionAtIndex(size_t transitionIndex)
	{
		return m_transitions.GetRefFromIndex(transitionIndex);
	}

	void SetEntity(EntityRef entity)
	{
		m_entity = entity;
	}
public:
	template <class Func>
	void IterateOnTransitions(Func function)
	{
		m_transitions.IterateConstRef(function);
	}

	template <class Func>
	void IterateOnTransitions(Func function) const
	{
		m_transitions.IterateConstRef(function);
	}
	
	template <class Func>
	void Tick(float currentSampleTime, float nextSampleTime, Func metachannelsCallback)
	{
		AnimationSimulator::Simulate(m_entity, m_animation, currentSampleTime, nextSampleTime, metachannelsCallback);
	}
private:
	stringType m_name;
	transitionContainerType m_transitions;
	toStateIndexesType m_toStateIndexes;
	AnimationRef m_animation;
	EntityRef m_entity;
};

}
