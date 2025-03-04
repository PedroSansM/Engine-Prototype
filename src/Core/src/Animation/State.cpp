#include "State.h"
#include "AssetManager.h"



namespace DCore
{

State::State(const stringType& name)
	:
	m_name(name)
{}

State::State(const State& other,
			integerParameterContainerType& integerParameters,
			floatParameterContainerType& floatParameters,
			logicParameterContainerType& logicParameters,
			triggerParameterContainerType& triggerParameters)
	:
	m_name(other.m_name),
	m_toStateIndexes(other.m_toStateIndexes),
	m_entity(other.m_entity)
{
	if (other.m_animation.IsValid())
	{
		m_animation = AssetManager::Get().GetAnimation(other.m_animation.GetUUID());
	}
	other.m_transitions.IterateConstRef
	(
		[&](transitionConstRefType transition) -> bool
		{	
			m_transitions.PushBack(Transition(*transition.Data(), integerParameters, floatParameters, logicParameters, triggerParameters));
			return false;
		}
	);
}

State::State(State&& other,
			integerParameterContainerType& integerParameters,
			floatParameterContainerType& floatParameters,
			logicParameterContainerType& logicParameters,
			triggerParameterContainerType& triggerParameters) noexcept
	:
	m_name(std::move(other.m_name)),
	m_toStateIndexes(std::move(other.m_toStateIndexes)),
	m_animation(other.m_animation),
	m_entity(other.m_entity)
{
	other.m_animation.Invalidate();
	other.m_transitions.IterateConstRef
	(
		[&](transitionConstRefType transition) -> bool
		{	
			m_transitions.PushBack(Transition(*transition.Data(), integerParameters, floatParameters, logicParameters, triggerParameters));
			return false;
		}
	);
}

State::State(State&& other) noexcept
	:
	m_name(std::move(other.m_name)),
	m_transitions(std::move(other.m_transitions)),
	m_toStateIndexes(std::move(other.m_toStateIndexes)),
	m_animation(other.m_animation),
	m_entity(other.m_entity)
{
	other.m_animation.Invalidate();
}

State::~State()
{
	m_animation.Unload();
}

void State::AddTransition(size_t fromStateIndex, size_t toStateIndex, transitionConstRefType* outTransition)
{
	DASSERT_E(!m_toStateIndexes.Exists(toStateIndex));
	transitionConstRefType transition(m_transitions.PushBackConst(fromStateIndex, toStateIndex));
	m_toStateIndexes.Add(toStateIndex);
	if (outTransition != nullptr)
	{
		*outTransition = transition;
	}
}

transitionConstRefType State::TryGetTransitionAtIndex(size_t index) const
{
	return m_transitions.GetRefFromIndex(index);
}

void State::DeleteTransitionAtIndex(size_t transitionIndex)
{
	transitionConstRefType transition(m_transitions.GetRefFromIndex(transitionIndex));
	DASSERT_E(transition.IsValid());
	m_toStateIndexes.Remove(transition->GetToStateIndex());
	m_transitions.RemoveElementAtIndex(transitionIndex);
}

void State::DeleteAllTransitionsThatGoToStateAtIndex(size_t stateIndex)
{
	m_transitions.IterateConstRef
	(
		[&](transitionContainerType::ConstRef transition) -> bool
		{
			if (transition->GetToStateIndex() == stateIndex)
			{
				m_toStateIndexes.Remove(transition->GetToStateIndex());
				m_transitions.Remove(transition);
			}
			return false;
		}
	);
}

bool State::TryMakeTransition(size_t& outToStateIndex)
{
	bool result(false);
	m_transitions.Iterate
	(
		[&](transitionRefType transition) -> bool
		{
			if (transition->ToExecute())
			{
				outToStateIndex = transition->GetToStateIndex();
				result = true;
				return true;
			}
			return false;
		}
	);
	return result;
}

}
