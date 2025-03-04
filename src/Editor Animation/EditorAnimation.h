#pragma once

#include "DommusCore.h"

#include <iterator>
#include <vector>
#include <algorithm>



namespace DEditor
{

// Samples per second
#define ANIMATION_SAMPLE_RATE 30

enum class KeyframeAddResult
{
	Ok,
	AlreadyAddedAtTime,
	KeyframeAtTimeZero
};

class IntegerKeyframe
{
public:
	using dVec2 = DCore::DVec2;
public:
	IntegerKeyframe();
	IntegerKeyframe(float time, int value);
	IntegerKeyframe(const IntegerKeyframe&);
	~IntegerKeyframe() = default;
public:
	dVec2 GetMainPoint() const
	{
		return {m_time, m_value};
	}
 
	void SetTime(float time)
	{
		m_time = time;
	}

	void SetValue(int value)
	{
		m_value = value;
	}

	float GetTime() const
	{
		return m_time;
	}

	int GetValue() const
	{
		return m_value;
	}
public:
	IntegerKeyframe& operator=(const IntegerKeyframe& other)
	{
		m_time = other.m_time;
		m_value = other.m_value;
		return *this;
	}
private:
	float m_time;
	int m_value;
};

class FloatKeyframe
{
public:
	using dVec2 = DCore::DVec2;
public:
	static constexpr float s_defaultTimeDifference{0.2f};
public:
	FloatKeyframe();
	FloatKeyframe(float time, float value);
	FloatKeyframe(const FloatKeyframe& other);
	~FloatKeyframe() = default;
public:
	dVec2 GetMainPoint() const
	{
		return {m_time, m_value};
	}

	void SetMainPoint(float time, float value)
	{
		float currentLeftControlPointTimeDifference(m_leftControlPointTime - m_time);
		float currentRightControlPointTimeDifference(m_rightControlPointTime - m_time);
		m_time = time;
		m_value = value;
		m_leftControlPointTime = m_time + currentLeftControlPointTimeDifference;
		m_rightControlPointTime = m_time + currentRightControlPointTimeDifference;
	}

	dVec2 GetLeftControlPoint() const
	{
		return {m_leftControlPointTime, m_leftControlPointValue};
	}

	void SetLeftControlPoint(float time, float value)
	{
		m_leftControlPointTime = std::min(time, m_time);
		m_leftControlPointValue = value;
	}

	dVec2 GetRightControlPoint() const
	{
		return {m_rightControlPointTime, m_rightControlPointValue};
	}

	void SetRightControlPoint(float time, float value)
	{
		m_rightControlPointTime = std::max(time, m_time);
		m_rightControlPointValue = value;
	}

	void SetTime(float time)
	{
		m_time = time;
	}

	float GetTime() const
	{
		return m_time;
	}

	float GetValue() const
	{
		return m_value;
	}
public:
	FloatKeyframe& operator=(const FloatKeyframe& other)
	{
		m_time = other.m_time;
		m_value = other.m_value;
		m_leftControlPointTime = other.m_leftControlPointTime;
		m_leftControlPointValue = other.m_leftControlPointValue;
		m_rightControlPointTime = other.m_rightControlPointTime;	
		m_rightControlPointValue = other.m_rightControlPointValue;
		return *this;
	}
private:
	float m_time;
	float m_value;
	float m_leftControlPointTime;
	float m_leftControlPointValue;
	float m_rightControlPointTime;
	float m_rightControlPointValue;
};

template <class KeyframeType>
struct KeyframeComparator
{
	bool operator()(const KeyframeType& a, const KeyframeType& b) const
	{
		return a.GetMainPoint().x < b.GetMainPoint().x;
	}
};

class IntegerAnimation
{
public:
	using integerKeyframeContainerType = std::vector<IntegerKeyframe>;
public:
	IntegerAnimation();
	IntegerAnimation(IntegerAnimation&&) noexcept;
	~IntegerAnimation() = default;
public:
	KeyframeAddResult TryAddKeyframe(const IntegerKeyframe&);
	void RemoveKeyframeAtIndex(size_t keyframeIndex);
	void SetKeyframeAtIndex(size_t keyframeIndex, float animationDuration, const IntegerKeyframe&);
	void SetDuration(float);
public:
	IntegerAnimation& operator=(IntegerAnimation&& other) noexcept
	{
		m_keyframes = std::move(other.m_keyframes);
		return *this;
	}
public:
	const integerKeyframeContainerType& GetKeyframes() const
	{
		return m_keyframes;
	}

	void Claar()
	{
		m_keyframes.clear();
	}
private:
	integerKeyframeContainerType m_keyframes;
};

class FloatAnimation
{
public:
	using floatKeyframeContainerType = std::vector<FloatKeyframe>;
public:
	FloatAnimation();
	FloatAnimation(FloatAnimation&&) noexcept;
	~FloatAnimation() = default;
public:
	KeyframeAddResult TryAddKeyframe(const FloatKeyframe&);
	void RemoveKeyframeAtIndex(size_t);
	void SetKeyframeAtIndex(size_t, float animationDuration, const FloatKeyframe&);
	void SetDuration(float);
	float Sample(float time) const;
public:
	const floatKeyframeContainerType& GetKeyframes() const
	{
		return m_keyframes;
	}

	void Clear()
	{
		m_keyframes.clear();
	}
public:
	FloatAnimation& operator=(FloatAnimation&& other) noexcept
	{
		m_keyframes = std::move(other.m_keyframes);
		return *this;
	}
private:
	floatKeyframeContainerType m_keyframes;
};

// E.g., sprite index attribute of Sprite Component will have a m_animations of size one.
// m_animations is indexed by the attribute component index (e.g., a vec3 have three components: x (id 0), y (id 1) and z (id 2)).
class IntegerAttributeAnimations
{
public:
	using integerAnimationContainerType = std::vector<IntegerAnimation>;
public:
	IntegerAttributeAnimations() = default;
	IntegerAttributeAnimations(IntegerAttributeAnimations&&) noexcept;
	~IntegerAttributeAnimations() = default;
public:
	void MakeAnimation(size_t numberOfComponents);
	KeyframeAddResult TryAddKeyframe(size_t attributeComponentId, const IntegerKeyframe&);
	void RemoveKeyframeAtIndex(size_t attributeComponentId, size_t index);
	void SetKeyframeAtIndex(size_t attributeComponentId, size_t keyframeIndex, float animationDuration, const IntegerKeyframe&);
public:
	const IntegerAnimation::integerKeyframeContainerType& GetKeyframes(size_t attributeComponentId) const
	{
		DASSERT_E(attributeComponentId < m_animations.size());
		return m_animations[attributeComponentId].GetKeyframes();
	}

	void Clear()
	{
		m_animations.clear();
	}

	void SetDuration(float duration)
	{
		for (IntegerAnimation& animation : m_animations)
		{
			animation.SetDuration(duration);
		}
	}
public:
	IntegerAttributeAnimations& operator=(IntegerAttributeAnimations&& other) noexcept
	{
		m_animations = std::move(other.m_animations);
		return *this;
	}
private:
	integerAnimationContainerType m_animations;
};

class FloatAttributeAnimations
{
public:
	using floatAnimationContainerType = std::vector<FloatAnimation>;
public:
	FloatAttributeAnimations() = default;
	FloatAttributeAnimations(FloatAttributeAnimations&&);
	~FloatAttributeAnimations() = default;
public:
	void MakeAnimation(size_t numberOfComponents);
	KeyframeAddResult TryAddKeyframe(size_t attributeComponentId, const FloatKeyframe&);
	void RemoveKeyframeAtIndex(size_t attributeComponentId, size_t index);
	void SetKeyframeAtIndex(size_t attributeComponentId, size_t keyframeIndex, float animationDuration, const FloatKeyframe&);
	float Sample(size_t attributeComponentId, float time) const;
public:
	const FloatAnimation::floatKeyframeContainerType& GetKeyframes(size_t attributeComponentId) const
	{
		DASSERT_E(attributeComponentId < m_animations.size());
		return m_animations[attributeComponentId].GetKeyframes();
	}

	void Clear()
	{
		m_animations.clear();
	}

	void SetDuration(float duration)
	{
		for (FloatAnimation& animation : m_animations)
		{
			animation.SetDuration(duration);
		}
	}
public:
	FloatAttributeAnimations& operator=(FloatAttributeAnimations && other) noexcept
	{
		m_animations = std::move(other.m_animations);
		return *this;
	}
private:
	floatAnimationContainerType m_animations;
};

class ComponentAnimations
{
public:
	using componentIdType = DCore::ComponentIdType;
	using attributeIdType = DCore::AttributeIdType;
	using attributeContainerType = DCore::SparseSet<attributeIdType>;
	using integerAttributeAnimationContainerType = std::vector<IntegerAttributeAnimations>;
	using floatAttributeAnimationContainerType = std::vector<FloatAttributeAnimations>;
public:
	ComponentAnimations() = default;
	ComponentAnimations(ComponentAnimations&&) noexcept;
	~ComponentAnimations() = default;
public:
	void MakeIntegerAnimation(attributeIdType attributeId, size_t numberOfComponents);
	void MakeFloatAnimation(attributeIdType attributeId, size_t numberOfComponents);
	void RemoveAnimation(componentIdType, attributeIdType);
	KeyframeAddResult TryAddIntegerKeyframe(attributeIdType attributeId, size_t attributeComponentId, const IntegerKeyframe&);
	KeyframeAddResult TryAddFloatKeyframe(attributeIdType attributeId, size_t attributeComponentId, const FloatKeyframe&);
	void RemoveIntegerKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index);
	void RemoveFloatKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index);
	bool IsAttributeAdded(attributeIdType attributeId) const;
	void SetIntegerKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index, float animationDuration, const IntegerKeyframe&);
	void SetFloatKeyframeAtIndex(attributeIdType attributeId, size_t attributeComponentId, size_t index, float animationDuration, const FloatKeyframe&);
	float Sample(attributeIdType attributeId, size_t attributeComponentId, float time) const;
public:
	const IntegerAnimation::integerKeyframeContainerType& GetIntegerKeyframes(attributeIdType attributeId, size_t attributeComponentId) const
	{
		DASSERT_E(m_attributes.Exists(attributeId));
		return m_integerAnimations[attributeId].GetKeyframes(attributeComponentId);
	}

	const FloatAnimation::floatKeyframeContainerType& GetFloatKeyframes(attributeIdType attributeId, size_t attributeComponentId) const
	{
		DASSERT_E(m_attributes.Exists(attributeId));
		return m_floatAnimations[attributeId].GetKeyframes(attributeComponentId);
	}

	const attributeContainerType::valueContainerType& GetAttributeIds() const
	{
		return m_attributes.GetDenseRef();
	}

	void SetDuration(float duration)
	{
		for (IntegerAttributeAnimations& animations : m_integerAnimations)
		{
			animations.SetDuration(duration);
		}
		for (FloatAttributeAnimations& animations : m_floatAnimations)
		{
			animations.SetDuration(duration);
		}
	}
public:
	ComponentAnimations& operator=(ComponentAnimations&& other) noexcept
	{
		m_integerAnimations = std::move(other.m_integerAnimations);
		m_floatAnimations = std::move(other.m_floatAnimations);
		m_attributes = std::move(other.m_attributes);
		return *this;
	}
private:
	integerAttributeAnimationContainerType m_integerAnimations;
	floatAttributeAnimationContainerType m_floatAnimations;
	attributeContainerType m_attributes;
private:
	template <class AttributeAnimationsType>
	void MakeAnimation(attributeIdType attributeId, size_t numberOfComponents, std::vector<AttributeAnimationsType>& animations)
	{
		DASSERT_E(!m_attributes.Exists(attributeId));
		if (attributeId >= animations.size())
		{
			animations.resize(attributeId + 1);
		}
		animations[attributeId].MakeAnimation(numberOfComponents);
		m_attributes.Add(attributeId);
	}

	template <class KeyframeType, class AttributeAnimationsType>
	KeyframeAddResult TryAddKeyframe(DCore::AttributeIdType attributeId, size_t attributeComponentId, const KeyframeType& keyframe, std::vector<AttributeAnimationsType>& animations)
	{
		DASSERT_E(m_attributes.Exists(attributeId));
		return animations[attributeId].TryAddKeyframe(attributeComponentId, keyframe);
	}

	template <class AttributeAnimationsType>
	void RemoveKeyframeAtIndex(DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t index, std::vector<AttributeAnimationsType>& animations)
	{
		DASSERT_E(m_attributes.Exists(attributeId));
		animations[attributeId].RemoveKeyframeAtIndex(attributeComponentId, index);
	}
};

class Animation
{
public:
	using integerKeyframeContainerType = IntegerAnimation::integerKeyframeContainerType;
	using floatKeyframeContainerType = FloatAnimation::floatKeyframeContainerType;
	using uuidType = DCore::UUIDType;
	using componentIdType = DCore::ComponentIdType;
	using attributeIdType = DCore::AttributeIdType;
	using componentContainerType = DCore::SparseSet<componentIdType>;
	using coreAnimationType = DCore::Animation;
	using metachannelContainerType = DCore::ReciclingVector<DCore::Metachannel>;
	using stringType = std::string;
	using animationContainerType = std::vector<ComponentAnimations>;
public:
	static constexpr float s_defaultDuration{60.0f}; // In seconds.
public:
	Animation(const uuidType&, const stringType& animationName, float duration);
	Animation(Animation&&) noexcept;
	~Animation() = default;
public:
	void MakeAnimation(componentIdType, attributeIdType);
	void RemoveAnimation(componentIdType, attributeIdType);
	bool IsAttributeOfComponentAdded(componentIdType, attributeIdType) const;
	KeyframeAddResult TryAddIntegerKeyframe(componentIdType, attributeIdType, size_t attributeComponentId, const IntegerKeyframe&);
	KeyframeAddResult TryAddFloatKeyframe(componentIdType, attributeIdType, size_t attributeComponentId, const FloatKeyframe&);
	void RemoveKeyframe(componentIdType, attributeIdType, size_t attributeComponentId, size_t keyframeIndex);
	void SetIntegerKeyframeAtIndex(componentIdType, attributeIdType, size_t attributeComponentId, size_t keyframeIndex, const IntegerKeyframe&);
	void SetFloatKeyframeAtIndex(componentIdType, attributeIdType, size_t attributeComponentId, size_t keyframeIndex, const FloatKeyframe&);
	coreAnimationType GenerateCoreAnimation() const;
	size_t GetNumberOfKeyframes(componentIdType, attributeIdType, size_t attributeComponentId) const;
	float Sample(componentIdType, attributeIdType, size_t attributeComponentId, float time) const;
	void AddMetachannel();
	void RemoveMetachannelAtIndex(size_t index);
public:
	const uuidType& GetUUID() const
	{
		return m_uuid;
	}

	const stringType& GetName() const
	{
		return m_name;
	}

	float GetDuration() const
	{
		return m_duration;
	}

	void SetDuration(float duration)
	{
		DASSERT_E(duration >= 0.0f);
		m_duration = duration;
		for (ComponentAnimations& componentAnimations : m_animations)
		{
			componentAnimations.SetDuration(duration);
		}
	}

	const integerKeyframeContainerType& GetIntegerKeyframes(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId) const
	{
		DASSERT_E(m_components.Exists(componentId));
		// TODO. Check if the attribute with id attributeId have float components
		return m_animations[componentId].GetIntegerKeyframes(attributeId, attributeComponentId);
	}

	const floatKeyframeContainerType& GetFloatKeyframes(componentIdType componentId, attributeIdType attributeId, size_t attributeComponentId) const
	{
		DASSERT_E(m_components.Exists(componentId));
		// TODO. Check if the attribute with id attributeId have float components
		return m_animations[componentId].GetFloatKeyframes(attributeId, attributeComponentId);
	}
public:
	template <class Func>
	void IterateOnComponentIdsAndAttributeIds(Func function)
	{
		for (componentIdType componentId : m_components.GetDenseRef())
		{
			if (std::invoke(function, componentId, m_animations[componentId].GetAttributeIds()))
			{
				return;
			}
		}
	}

	template <class Func>
	void IterateOnComponentIdsAndAttributeIds(Func function) const
	{
		for (DCore::ComponentIdType componentId : m_components.GetDenseRef())
		{
			if (std::invoke(function, componentId, m_animations[componentId].GetAttributeIds()))
			{
				return;
			}
		}
	}

	template <class Func>
	void IterateOnMetachannels(Func function)
	{
		m_metachannels.Iterate
		(
			[&](metachannelContainerType::Ref metachannel) -> bool
			{
				return std::invoke(function, metachannel);
			}
		);
	}

	template <class Func>
	void IterateOnMetachannels(Func function) const
	{
		m_metachannels.Iterate
		(
			[&](metachannelContainerType::ConstRef metachannel) -> bool
			{
				return std::invoke(function, metachannel);
			}
		);
	}
private:
	uuidType m_uuid;
	stringType m_name;
	float m_duration;
	animationContainerType m_animations;
 	componentContainerType m_components;
	metachannelContainerType m_metachannels;
};

}
