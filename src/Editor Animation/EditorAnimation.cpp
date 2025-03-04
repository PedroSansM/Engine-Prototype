#include "EditorAnimation.h"

#include <algorithm>
#include <iterator>



namespace DEditor
{

IntegerKeyframe::IntegerKeyframe()
	:
	m_time(0.0f),
	m_value(0.0f)
{}

IntegerKeyframe::IntegerKeyframe(float time, int value)
	:
	m_time(time),
	m_value(value)
{}

IntegerKeyframe::IntegerKeyframe(const IntegerKeyframe& other)
	:
	m_time(other.m_time),
	m_value(other.m_value)
{}

FloatKeyframe::FloatKeyframe()
	:
	m_time(0.0f),
	m_value(0.0f),
	m_leftControlPointTime(m_time - s_defaultTimeDifference),
	m_leftControlPointValue(m_value),
	m_rightControlPointTime(m_time + s_defaultTimeDifference),
	m_rightControlPointValue(m_value)
{}

FloatKeyframe::FloatKeyframe(float time, float value)
	:
	m_time(time),
	m_value(value),
	m_leftControlPointTime(time - s_defaultTimeDifference),
	m_leftControlPointValue(value),
	m_rightControlPointTime(time + s_defaultTimeDifference),
	m_rightControlPointValue(value)
{}

FloatKeyframe::FloatKeyframe(const FloatKeyframe& other)
	:
	m_time(other.m_time),
	m_value(other.m_value),
	m_leftControlPointTime(other.m_leftControlPointTime),
	m_leftControlPointValue(other.m_leftControlPointValue),
	m_rightControlPointTime(other.m_rightControlPointTime),
	m_rightControlPointValue(other.m_rightControlPointValue)
{}

IntegerAnimation::IntegerAnimation()
{
	m_keyframes.push_back({0.0f, 0});
}

IntegerAnimation::IntegerAnimation(IntegerAnimation&& other) noexcept
	:
	m_keyframes(std::move(other.m_keyframes))
{}

KeyframeAddResult IntegerAnimation::TryAddKeyframe(const IntegerKeyframe& keyframe)
{
	if (keyframe.GetMainPoint().x == 0.0f)
	{
		return KeyframeAddResult::KeyframeAtTimeZero;
	}
	for (const IntegerKeyframe& integerKeyframe : m_keyframes)
	{
		if (integerKeyframe.GetMainPoint().x == keyframe.GetMainPoint().x)
		{
			return KeyframeAddResult::AlreadyAddedAtTime;
		}
	}
	m_keyframes.push_back(keyframe);
	std::sort(m_keyframes.begin(), m_keyframes.end(), KeyframeComparator<IntegerKeyframe>{});
	return KeyframeAddResult::Ok;
}

void IntegerAnimation::RemoveKeyframeAtIndex(size_t index)
{
	DASSERT_E(index < m_keyframes.size());
	auto it(m_keyframes.begin() + index);
	m_keyframes.erase(it);
}

void IntegerAnimation::SetKeyframeAtIndex(size_t keyframeIndex, float animationDuration, const IntegerKeyframe& keyframe)
{
	DASSERT_E(keyframeIndex < m_keyframes.size());
	IntegerKeyframe keyframeToAdd(keyframe);
	if (keyframeIndex == 0 && keyframe.GetMainPoint().x != 0)
	{
		keyframeToAdd.SetTime(0.0f);
	}
	if (keyframe.GetMainPoint().x > animationDuration)
	{
		keyframeToAdd.SetTime(animationDuration);
	}
	if (keyframeIndex - 1 < m_keyframes.size() && keyframe.GetMainPoint().x <= m_keyframes[keyframeIndex - 1].GetMainPoint().x)
	{
		keyframeToAdd.SetTime(m_keyframes[keyframeIndex - 1].GetMainPoint().x + 1.0f / ANIMATION_SAMPLE_RATE);
	}
	else if (keyframeIndex + 1 < m_keyframes.size() && keyframe.GetMainPoint().x >= m_keyframes[keyframeIndex + 1].GetMainPoint().x)
	{
		keyframeToAdd.SetTime(m_keyframes[keyframeIndex + 1].GetMainPoint().x - 1.0f / ANIMATION_SAMPLE_RATE);
	}
	m_keyframes[keyframeIndex] = keyframeToAdd;
}

void IntegerAnimation::SetDuration(float duration)
{
	for (auto it(m_keyframes.begin()); it != m_keyframes.end(); it++)
	{
		if (it->GetMainPoint().x > duration)
		{
			m_keyframes.erase(it, m_keyframes.end());
			return;
		}
	}
}

FloatAnimation::FloatAnimation()
{
	m_keyframes.push_back({0.0f, 0.0f});
}

FloatAnimation::FloatAnimation(FloatAnimation&& other) noexcept
	:
	m_keyframes(std::move(other.m_keyframes))
{}

KeyframeAddResult FloatAnimation::TryAddKeyframe(const FloatKeyframe& inKeyframe)
{
	if (inKeyframe.GetMainPoint().x == 0.0f)
	{
		return KeyframeAddResult::KeyframeAtTimeZero;
	}
	for (const FloatKeyframe& keyframe : m_keyframes)
	{
		if (keyframe.GetMainPoint().x == inKeyframe.GetMainPoint().x)
		{
			return KeyframeAddResult::AlreadyAddedAtTime;
		}
	}
	m_keyframes.push_back(inKeyframe);
	std::sort(m_keyframes.begin(), m_keyframes.end(), KeyframeComparator<FloatKeyframe>{});
	for (size_t index(0); index < m_keyframes.size(); index++)
	{
		if (m_keyframes[index].GetMainPoint().x != inKeyframe.GetMainPoint().x)
		{
			continue;
		}
		FloatKeyframe& addedKeyframe(m_keyframes[index]);
		FloatKeyframe& keyframeAtLeft(m_keyframes[index - 1]);
		if (keyframeAtLeft.GetRightControlPoint().x > addedKeyframe.GetMainPoint().x)
		{
			keyframeAtLeft.SetRightControlPoint(addedKeyframe.GetMainPoint().x, keyframeAtLeft.GetRightControlPoint().y);
		}
		if (addedKeyframe.GetLeftControlPoint().x < keyframeAtLeft.GetMainPoint().x)
		{
			addedKeyframe.SetLeftControlPoint(keyframeAtLeft.GetMainPoint().x, addedKeyframe.GetLeftControlPoint().y);
		}
		if (index + 1 < m_keyframes.size())
		{
			FloatKeyframe& keyframeAtRight(m_keyframes[index + 1]);
			if (keyframeAtRight.GetLeftControlPoint().x < addedKeyframe.GetMainPoint().x)
			{
				keyframeAtRight.SetLeftControlPoint(addedKeyframe.GetMainPoint().x, keyframeAtRight.GetLeftControlPoint().y);
			}
			if (addedKeyframe.GetRightControlPoint().x > keyframeAtRight.GetMainPoint().x)
			{
				addedKeyframe.SetRightControlPoint(keyframeAtRight.GetMainPoint().x, addedKeyframe.GetRightControlPoint().y);
			}
		}
		break;
	}
	return KeyframeAddResult::Ok;
}

void FloatAnimation::RemoveKeyframeAtIndex(size_t index)
{
	DASSERT_E(index < m_keyframes.size());
	auto it(m_keyframes.begin());
	std::advance(it, index);
	m_keyframes.erase(it);
}

void FloatAnimation::SetKeyframeAtIndex(size_t keyframeIndex, float animationDuration, const FloatKeyframe& keyframe)
{
	DASSERT_E(keyframeIndex < m_keyframes.size());
	FloatKeyframe keyframeToAdd(keyframe);
	if (keyframeIndex > 0)
	{
		FloatKeyframe& leftKeyframe(m_keyframes[keyframeIndex - 1]);
		if (keyframeToAdd.GetLeftControlPoint().x < leftKeyframe.GetMainPoint().x)
		{
			keyframeToAdd.SetLeftControlPoint(leftKeyframe.GetMainPoint().x, keyframeToAdd.GetLeftControlPoint().y);
		}
		if (leftKeyframe.GetRightControlPoint().x > keyframeToAdd.GetMainPoint().x)
		{
			leftKeyframe.SetRightControlPoint(keyframeToAdd.GetMainPoint().x, leftKeyframe.GetRightControlPoint().y);
		}
	}
	if (keyframeIndex + 1 < m_keyframes.size())
	{
		FloatKeyframe& rightKeyframe(m_keyframes[keyframeIndex + 1]);
		if (keyframeToAdd.GetRightControlPoint().x > rightKeyframe.GetMainPoint().x)
		{
			keyframeToAdd.SetRightControlPoint(rightKeyframe.GetMainPoint().x, keyframeToAdd.GetRightControlPoint().y);
		}
		if (rightKeyframe.GetLeftControlPoint().x < keyframeToAdd.GetMainPoint().x)
		{
			rightKeyframe.SetLeftControlPoint(keyframeToAdd.GetMainPoint().x, rightKeyframe.GetLeftControlPoint().y);
		}
	}
	if (keyframeIndex == 0 && keyframeToAdd.GetMainPoint().x != 0)
	{
		keyframeToAdd.SetMainPoint(0.0f, keyframeToAdd.GetMainPoint().y);
	}
	else if (keyframeIndex + 1 < m_keyframes.size() && m_keyframes[keyframeIndex + 1].GetMainPoint().x <= keyframeToAdd.GetMainPoint().x)
	{
		keyframeToAdd.SetMainPoint(m_keyframes[keyframeIndex + 1].GetMainPoint().x - 1.0f / ANIMATION_SAMPLE_RATE, keyframeToAdd.GetMainPoint().y);
	}
	else if (keyframeIndex - 1 < m_keyframes.size() && m_keyframes[keyframeIndex - 1].GetMainPoint().x >= keyframeToAdd.GetMainPoint().x)
	{
		keyframeToAdd.SetMainPoint(m_keyframes[keyframeIndex - 1].GetMainPoint().x + 1.0f / ANIMATION_SAMPLE_RATE, keyframeToAdd.GetMainPoint().y);
	}
	else if (keyframe.GetMainPoint().x > animationDuration)
	{
		keyframeToAdd.SetMainPoint(animationDuration, keyframeToAdd.GetMainPoint().y);
	}
	m_keyframes[keyframeIndex] = keyframeToAdd;
}

void FloatAnimation::SetDuration(float duration)
{
	for (auto it(m_keyframes.begin()); it != m_keyframes.end(); it++)
	{
		if (it->GetMainPoint().x > duration)
		{
			m_keyframes.erase(it, m_keyframes.end());
			return;
		}
	}
}

float FloatAnimation::Sample(float time) const
{
	using dVec2 = DCore::DVec2;
	DASSERT_E(time >= 0.0f);
	DASSERT_E(m_keyframes.size() > 0);
	if (time == 0.0f)
	{
		return m_keyframes[0].GetValue();
	}
	if (time >= m_keyframes[m_keyframes.size() - 1].GetTime())
	{
		return m_keyframes[m_keyframes.size() - 1].GetValue();
	}
	float returnValue(0.0f);
	for (size_t index(0); index < m_keyframes.size() - 1; index++)
	{
		if (m_keyframes[index].GetTime() == time)
		{
			return m_keyframes[index].GetValue();
		}
		if (m_keyframes[index + 1].GetTime() == time)
		{
			return m_keyframes[index + 1].GetValue();
		}
		if (time > m_keyframes[index].GetTime() && time < m_keyframes[index + 1].GetTime())
		{
			const dVec2 p1(m_keyframes[index].GetMainPoint());
			const dVec2 p2(m_keyframes[index].GetRightControlPoint());
			const dVec2 p3(m_keyframes[index + 1].GetLeftControlPoint());
			const dVec2 p4(m_keyframes[index + 1].GetMainPoint());
			const float t((time - p1.x)/(p4.x - p1.x));
			returnValue = pow(1-t, 3)*p1.y + 3*t*pow(1-t, 2)*p2.y + 3*pow(t, 2)*(1-t)*p3.y + pow(t, 3)*p4.y;
			break;
		}
	}
	return returnValue;
}

IntegerAttributeAnimations::IntegerAttributeAnimations(IntegerAttributeAnimations&& other) noexcept
	:
	m_animations(std::move(other.m_animations))
{}

void IntegerAttributeAnimations::MakeAnimation(size_t numberOfComponents)
{
	m_animations.clear();
	m_animations.resize(numberOfComponents);
}

KeyframeAddResult IntegerAttributeAnimations::TryAddKeyframe(size_t attributeComponentId, const IntegerKeyframe& keyframe)
{
	DASSERT_E(attributeComponentId < m_animations.size());
	return m_animations[attributeComponentId].TryAddKeyframe(keyframe);
}

void IntegerAttributeAnimations::RemoveKeyframeAtIndex(size_t attributeComponentId, size_t index)
{
	DASSERT_E(attributeComponentId < m_animations.size());
	m_animations[attributeComponentId].RemoveKeyframeAtIndex(index);
}

void IntegerAttributeAnimations::SetKeyframeAtIndex(size_t attributeComponentId, size_t keyframeIndex, float animationDuration, const IntegerKeyframe& keyframe)
{
	DASSERT_E(attributeComponentId < m_animations.size());
	m_animations[attributeComponentId].SetKeyframeAtIndex(keyframeIndex, animationDuration, keyframe);
}

FloatAttributeAnimations::FloatAttributeAnimations(FloatAttributeAnimations&& other)
	:
	m_animations(std::move(other.m_animations))
{}

void FloatAttributeAnimations::MakeAnimation(size_t numberOfComponents)
{
	m_animations.clear();
	m_animations.resize(numberOfComponents);
}

KeyframeAddResult FloatAttributeAnimations::TryAddKeyframe(size_t attributeComponentId, const FloatKeyframe& keyframe)
{
	DASSERT_E(attributeComponentId < m_animations.size());
	return m_animations[attributeComponentId].TryAddKeyframe(keyframe);
}

void FloatAttributeAnimations::RemoveKeyframeAtIndex(size_t attributeComponentId, size_t index)
{
	DASSERT_E(attributeComponentId < m_animations.size());
	m_animations[attributeComponentId].RemoveKeyframeAtIndex(index);
}

void FloatAttributeAnimations::SetKeyframeAtIndex(size_t attributeComponentId, size_t keyframeIndex, float animationDuration, const FloatKeyframe& keyframe)
{
	DASSERT_E(attributeComponentId < m_animations.size());
	m_animations[attributeComponentId].SetKeyframeAtIndex(keyframeIndex, animationDuration, keyframe);
}

float FloatAttributeAnimations::Sample(size_t attributeComponentId, float time) const
{
	DASSERT_E(attributeComponentId < m_animations.size());
	return m_animations[attributeComponentId].Sample(time);
}

ComponentAnimations::ComponentAnimations(ComponentAnimations&& other) noexcept
	:
	m_integerAnimations(std::move(other.m_integerAnimations)),
	m_floatAnimations(std::move(other.m_floatAnimations)),
	m_attributes(std::move(other.m_attributes))
{}

void ComponentAnimations::MakeIntegerAnimation(attributeIdType attributeId, size_t numberOfComponents)
{
	MakeAnimation(attributeId, numberOfComponents, m_integerAnimations);
}

void ComponentAnimations::MakeFloatAnimation(attributeIdType attributeId, size_t numberOfComponents)
{
	MakeAnimation(attributeId, numberOfComponents, m_floatAnimations);
}

void ComponentAnimations::RemoveAnimation(componentIdType componentId, attributeIdType attributeId)
{
	DASSERT_E(m_attributes.Exists(attributeId));
	m_attributes.Remove(attributeId);
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	const DCore::AttributeKeyframeType attributeKeyframeType(componentForm.SerializedAttributes[attributeId].GetKeyframeType());
	DASSERT_E(attributeKeyframeType != DCore::AttributeKeyframeType::NotDefined);
	if (attributeKeyframeType == DCore::AttributeKeyframeType::Integer)
	{
		m_integerAnimations[attributeId].Clear();
		return;
	}
	m_floatAnimations[attributeId].Clear();
}

KeyframeAddResult ComponentAnimations::TryAddIntegerKeyframe(attributeIdType attributeId, size_t attributeComponentId, const IntegerKeyframe& keyframe)
{
	return TryAddKeyframe(attributeId, attributeComponentId, keyframe, m_integerAnimations);
}

KeyframeAddResult ComponentAnimations::TryAddFloatKeyframe(attributeIdType attributeId, size_t attributeComponentId, const FloatKeyframe& keyframe)
{
	return TryAddKeyframe(attributeId, attributeComponentId, keyframe, m_floatAnimations);
}

void ComponentAnimations::RemoveIntegerKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index)
{
	RemoveKeyframeAtIndex(attributeId, attributeComponentId, index, m_integerAnimations);
}

void ComponentAnimations::RemoveFloatKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index)
{
	RemoveKeyframeAtIndex(attributeId, attributeComponentId, index, m_floatAnimations);
}

bool ComponentAnimations::IsAttributeAdded(attributeIdType attributeId) const
{
	return m_attributes.Exists(attributeId);
}

void ComponentAnimations::SetIntegerKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index, float animationDuration, const IntegerKeyframe& keyframe)
{
	DASSERT_E(m_attributes.Exists(attributeId));
	m_integerAnimations[attributeId].SetKeyframeAtIndex(attributeComponentId, index, animationDuration, keyframe);
}

void ComponentAnimations::SetFloatKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex, float animationDuration, const FloatKeyframe& keyframe)
{
	DASSERT_E(m_attributes.Exists(attributeId));
	m_floatAnimations[attributeId].SetKeyframeAtIndex(attributeComponentId, keyframeIndex, animationDuration, keyframe);
}

float ComponentAnimations::Sample(attributeIdType attributeId, size_t attributeComponentId, float time) const
{
	DASSERT_E(m_attributes.Exists(attributeId));
	return m_floatAnimations[attributeId].Sample(attributeComponentId, time);
}

Animation::Animation(const uuidType& uuid, const stringType& animationName, float duration)
	:
	m_uuid(uuid),
	m_name(animationName),
	m_duration(duration)
{}

Animation::Animation(Animation&& other) noexcept
	:
	m_uuid(other.m_uuid),
	m_name(std::move(other.m_name)),
	m_duration(other.m_duration),
	m_animations(std::move(other.m_animations)),
	m_components(std::move(other.m_components)),
	m_metachannels(std::move(other.m_metachannels))
{}

void Animation::MakeAnimation(componentIdType componentId, attributeIdType attributeId)
{
	if (!m_components.Exists(componentId))
	{
		m_components.Add(componentId);
	}
	if (componentId >= m_animations.size())
	{
		m_animations.resize(componentId + 1);
	}
	DASSERT_E(DCore::ComponentForms::Get().ExistsComponentWithAttributeId(componentId, attributeId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	const DCore::AttributeKeyframeType attributeKeyframeType(componentForm.SerializedAttributes[attributeId].GetKeyframeType());
	DASSERT_E(attributeKeyframeType != DCore::AttributeKeyframeType::NotDefined);
	const size_t numberOfAttributeComponents(componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents());
	if (attributeKeyframeType == DCore::AttributeKeyframeType::Integer)
	{
		m_animations[componentId].MakeIntegerAnimation(attributeId, numberOfAttributeComponents);
		return;
	}
	m_animations[componentId].MakeFloatAnimation(attributeId, numberOfAttributeComponents);
}

void Animation::RemoveAnimation(componentIdType componentId, attributeIdType attributeId)
{
	DASSERT_E(m_components.Exists(componentId));
	m_animations[componentId].RemoveAnimation(componentId, attributeId);
	if (m_animations[componentId].GetAttributeIds().size() == 0)
	{
		m_components.Remove(componentId);
	}
}

bool Animation::IsAttributeOfComponentAdded(componentIdType componentId, attributeIdType attributeId) const
{
	return m_components.Exists(componentId) && m_animations[componentId].IsAttributeAdded(attributeId);
}

KeyframeAddResult Animation::TryAddIntegerKeyframe(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId, const IntegerKeyframe& keyframe)
{
	DASSERT_E(IsAttributeOfComponentAdded(componentId, attributeId));
	DASSERT_E(DCore::ComponentForms::Get().ExistsComponentWithAttributeId(componentId, attributeId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	DASSERT_E(componentForm.SerializedAttributes[attributeId].GetKeyframeType() == DCore::AttributeKeyframeType::Integer); 
	return m_animations[componentId].TryAddIntegerKeyframe(attributeId, attributeComponentId, keyframe);
}

KeyframeAddResult Animation::TryAddFloatKeyframe(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId, const FloatKeyframe& keyframe)
{
	DASSERT_E(IsAttributeOfComponentAdded(componentId, attributeId));
	DASSERT_E(DCore::ComponentForms::Get().ExistsComponentWithAttributeId(componentId, attributeId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	DASSERT_E(componentForm.SerializedAttributes[attributeId].GetKeyframeType() == DCore::AttributeKeyframeType::Float);
	return m_animations[componentId].TryAddFloatKeyframe(attributeId, attributeComponentId, keyframe);
}


void Animation::RemoveKeyframe(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex)
{
	DASSERT_E(keyframeIndex > 0);
	DASSERT_E(IsAttributeOfComponentAdded(componentId, attributeId));
	DASSERT_E(DCore::ComponentForms::Get().ExistsComponentWithAttributeId(componentId, attributeId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	if (componentForm.SerializedAttributes[attributeId].GetKeyframeType() == DCore::AttributeKeyframeType::Float)
	{
		m_animations[componentId].RemoveFloatKeyframeAtIndex(attributeId, attributeComponentId, keyframeIndex);
		return;
	}
	m_animations[componentId].RemoveIntegerKeyframeAtIndex(attributeId, attributeComponentId, keyframeIndex);
}

void Animation::SetIntegerKeyframeAtIndex(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex, const IntegerKeyframe& keyframe)
{
	DASSERT_E(IsAttributeOfComponentAdded(componentId, attributeId));
	DASSERT_E(DCore::ComponentForms::Get().ExistsComponentWithAttributeId(componentId, attributeId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	DASSERT_E(componentForm.SerializedAttributes[attributeId].GetKeyframeType() == DCore::AttributeKeyframeType::Integer); 
	m_animations[componentId].SetIntegerKeyframeAtIndex(attributeId, attributeComponentId, keyframeIndex, m_duration, keyframe);
}

void Animation::SetFloatKeyframeAtIndex(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex, const FloatKeyframe& keyframe)
{
	DASSERT_E(m_components.Exists(componentId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	DASSERT_E(componentForm.SerializedAttributes[attributeId].GetKeyframeType() == DCore::AttributeKeyframeType::Float);
	m_animations[componentId].SetFloatKeyframeAtIndex(attributeId, attributeComponentId, keyframeIndex, m_duration, keyframe);
}

Animation::coreAnimationType Animation::GenerateCoreAnimation() const
{
	using componentFormType = DCore::ComponentForm;
	using serializedAttributeType = DCore::SerializedAttribute;
	using attributeKeyframeType = DCore::AttributeKeyframeType;
	using coreIntegerKeyframeType = DCore::Keyframe<int>;
	using coreFloatKeyframeType = DCore::Keyframe<float>;
	coreAnimationType animation(m_name, m_duration);
	for (componentIdType componentId : m_components)
	{
		const ComponentAnimations& componentAnimations(m_animations[componentId]);
		for (ComponentAnimations::attributeIdType attributeId : componentAnimations.GetAttributeIds())
		{
			animation.MakeAnimation(componentId, attributeId);
			const componentFormType& componentForm(DCore::ComponentForms::Get()[componentId]);
			const serializedAttributeType& serializedAttribute(componentForm.SerializedAttributes[attributeId]);
			for (size_t attributeComponentId(0); attributeComponentId < serializedAttribute.GetNumberOfAttributeComponents(); attributeComponentId++)
			{
				switch (serializedAttribute.GetKeyframeType())
				{
				case attributeKeyframeType::Integer:
				{
					const IntegerAnimation::integerKeyframeContainerType& keyframes(componentAnimations.GetIntegerKeyframes(attributeId, attributeComponentId));
					for (const IntegerAnimation::integerKeyframeContainerType::value_type& keyframe : keyframes)
					{
						animation.AddKeyframe(componentId, attributeId, attributeComponentId, coreIntegerKeyframeType(keyframe.GetTime(), keyframe.GetValue()));
					}
					break;
				}
				case attributeKeyframeType::Float:
				{
					for (float time(0); time <= m_duration; time += 1.0f / coreAnimationType::sampleRate)
					{
						float value(Sample(componentId, attributeId, attributeComponentId, time));
						animation.AddKeyframe(componentId, attributeId, attributeComponentId, coreFloatKeyframeType(time, value));
					}
					break;
				}
				default:
					break;
				}
			}
		}
	}
	m_metachannels.Iterate
	(
		[&](auto metachannel) -> bool
		{
			auto coreAnimationMetachannel(animation.MakeMetachannel());
			coreAnimationMetachannel->SetId(metachannel->GetId());
			coreAnimationMetachannel->SetTime(metachannel->GetTime());
			return false;
		}
	);
	return animation;
}

size_t Animation::GetNumberOfKeyframes(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId) const
{
	DASSERT_E(m_components.Exists(componentId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	DASSERT_E(attributeId < componentForm.SerializedAttributes.size());	
	const DCore::SerializedAttribute& attribute(componentForm.SerializedAttributes[attributeId]);
	DASSERT_E(attribute.GetKeyframeType() != DCore::AttributeKeyframeType::NotDefined);
	if (attribute.GetKeyframeType() == DCore::AttributeKeyframeType::Integer)
	{
		return GetIntegerKeyframes(componentId, attributeId, attributeComponentId).size();
	}
	return GetFloatKeyframes(componentId, attributeId, attributeComponentId).size();
}

float Animation::Sample(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId, float time) const
{
	DASSERT_E(m_components.Exists(componentId));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	DASSERT_E(componentForm.SerializedAttributes[attributeId].GetKeyframeType() == DCore::AttributeKeyframeType::Float);
	return m_animations[componentId].Sample(attributeId, attributeComponentId, time);
}

void Animation::AddMetachannel()
{
	m_metachannels.PushBack();
}

void Animation::RemoveMetachannelAtIndex(size_t index)
{
	m_metachannels.RemoveElementAtIndex(index);
}

}
