#include "Animation.h"
#include "DCoreAssert.h"
#include "AssetManager.h"
#include "AssetManagerTypes.h"
#include "ComponentForm.h"
#include "ReadWriteLockGuard.h"
#include "SerializationTypes.h"

#include <algorithm>
#include <tuple>



namespace DCore
{

// ComponentAnimations
ComponentAnimations::ComponentAnimations(ComponentAnimations&& other) noexcept
	:
	m_integerAnimations(std::move(other.m_integerAnimations)),
	m_floatAnimations(std::move(other.m_floatAnimations)),
	m_attributeIndexes(std::move(other.m_attributeIndexes))
{}

ComponentAnimations& ComponentAnimations::operator=(ComponentAnimations&& other) noexcept
{
	m_integerAnimations = std::move(other.m_integerAnimations);
	m_floatAnimations = std::move(other.m_floatAnimations);
	m_attributeIndexes = std::move(other.m_attributeIndexes);
	return *this;
}
// End ComponentAnimations

// Animation
Animation::Animation(const stringType& name, float duration)
	:
	m_name(name),
	m_duration(duration)
{}

Animation::Animation(Animation&& other) noexcept
	:
	m_name(std::move(other.m_name)),
	m_duration(other.m_duration),
	m_componentAnimations(std::move(other.m_componentAnimations)),
	m_componentIndexes(std::move(other.m_componentIndexes)),
	m_metachannels(std::move(other.m_metachannels))
{}

void Animation::MakeAnimation(ComponentIdType componentIndex, AttributeIdType attributeIndex)
{
	DASSERT_E(ComponentForms::Get().ExistsComponentWithId(componentIndex));
	DASSERT_E(ComponentForms::Get().ExistsComponentWithAttributeId(componentIndex, attributeIndex));
	const ComponentForm& componentForm(ComponentForms::Get()[componentIndex]);
	const SerializedAttribute& attribute(componentForm.SerializedAttributes[attributeIndex]);
	const AttributeKeyframeType keyframeType(attribute.GetKeyframeType());
	DASSERT_E(keyframeType != AttributeKeyframeType::NotDefined);
	if (!m_componentIndexes.Exists(componentIndex))
	{
		m_componentIndexes.Add(componentIndex);
	}
	if (componentIndex >= m_componentAnimations.size())
	{
		m_componentAnimations.resize(componentIndex + 1);
	}
	if (keyframeType == AttributeKeyframeType::Integer)
	{
		m_componentAnimations[componentIndex].MakeAttributeAnimation<int>(attributeIndex, attribute.GetNumberOfAttributeComponents());
		return;
	}
	m_componentAnimations[componentIndex].MakeAttributeAnimation<float>(attributeIndex, attribute.GetNumberOfAttributeComponents());
}

Animation::metachannelContainerType::Ref Animation::MakeMetachannel()
{
	return m_metachannels.PushBack({});
}

void Animation::RemoveMetachannelAtIndex(size_t index)
{
	m_metachannels.RemoveElementAtIndex(index);
}

Animation& Animation::operator=(Animation&& other) noexcept
{
	m_name = std::move(other.m_name);
	m_duration = other.m_duration;
	m_componentAnimations = std::move(other.m_componentAnimations);
	m_componentIndexes = std::move(other.m_componentIndexes);
	m_metachannels = std::move(other.m_metachannels);
	return *this;
}
// End Animation

// AnimationRef
AnimationRef::AnimationRef()
	:
	m_lockData(nullptr)
{}

AnimationRef::AnimationRef(InternalAnimationRefType ref, LockData& lockData)
	:
	m_ref(ref),
	m_lockData(&lockData)
{}

AnimationRef::AnimationRef(const AnimationRef& other)
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{}

bool AnimationRef::IsValid() const
{
	return m_lockData != nullptr && m_ref.IsValid();
}

AnimationRef::stringType AnimationRef::GetName() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetName();
}

float AnimationRef::GetDuration() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetDuration();
}

UUIDType AnimationRef::GetUUID() const
{
	DASSERT_E(IsValid());
	return m_ref->GetUUID();
}

void AnimationRef::Invalidate()
{
	m_ref.Invalidate();
}

void AnimationRef::Unload()
{
	if (IsValid())
	{
		DCore::AssetManager::Get().UnloadAnimation(m_ref->GetUUID());
	}
}

AnimationRef& AnimationRef::operator=(Animation&& animation) noexcept
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset() = std::move(animation);
	return *this;
}
// End AnimationRef

}
