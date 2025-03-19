#pragma once

#include "ECSTypes.h"
#include "SparseSet.h"
#include "AssetManagerTypes.h"
#include "ReadWriteLockGuard.h"
#include "Asset.h"
#include "DCoreAssert.h"
#include "SerializationTypes.h"
#include "ComponentForm.h"

#include <type_traits>
#include <vector>
#include <string>

// Runtime animation definition.



namespace DCore
{

template <class ValueType>
class Keyframe
{
	static_assert(std::is_same_v<int, ValueType> || std::is_same_v<float, ValueType>, "Only value types of integer and float are allowed to animation keyframes.");
public:
	using valueType = ValueType;
public:
	Keyframe()
		:
		m_time(0.0f),
		m_value(0.0f)
	{}
	Keyframe(float time, float value)
		:
		m_time(time),
		m_value(value)
	{}
	Keyframe(const Keyframe& other)
		:
		m_time(other.m_time),
		m_value(other.m_value)
	{}
	~Keyframe() = default;
public:
	float GetTime() const
	{
		return m_time;
	}
	
	float GetValue() const
	{
		return m_value;
	}
private:
	float m_time;
	valueType m_value;
};

template <class KeyframeValueType>
struct KeyframeComparator
{
	bool operator()(const Keyframe<KeyframeValueType>& a, const Keyframe<KeyframeValueType>& b) const
	{
		return a.GetTime() < b.GetTime();
	}
};

// E.g., pode ser a animação da componente x do attributo "posição" do componente Transform.
// E.g., can be the animation of the component x of attribute "position" of component Transform.
template <class KeyframeValueType>
class AttributeAnimation
{
	static_assert(std::is_same_v<int, KeyframeValueType> || std::is_same_v<float, KeyframeValueType>, "Invalid component animation type.");
public:
	using keyframeValueType = KeyframeValueType;
	using keyframeType = Keyframe<keyframeValueType>;
	using keyframeContainerType = std::vector<keyframeType>;
public:
	AttributeAnimation() = default;
	AttributeAnimation(const AttributeAnimation& other)
		:
		m_keyframes(other.m_keyframes)
	{}
	AttributeAnimation(AttributeAnimation&& other) noexcept
		:
		m_keyframes(std::move(other.m_keyframes))
	{}
	~AttributeAnimation() = default;
public:
	void AddKeyframe(const keyframeType& keyframe)
	{
		m_keyframes.push_back(keyframe);
		std::sort(m_keyframes.begin(), m_keyframes.end(), KeyframeComparator<keyframeValueType>{});
	}

	keyframeValueType Sample(float sampleTime) const
	{
		if constexpr (std::is_same_v<int, KeyframeValueType>)
		{
			for (size_t index(0); index < m_keyframes.size(); index++)
			{
				if (m_keyframes[index].GetTime() == sampleTime)
				{
					return m_keyframes[index].GetValue();
				}
				if (m_keyframes[index].GetTime() > sampleTime)
				{
					return m_keyframes[index - 1].GetValue();
				}
			}
			return m_keyframes[m_keyframes.size() - 1].GetValue();
		}
		else
		{
			for (size_t index(0); index < m_keyframes.size(); index++)
			{
				if (m_keyframes[index].GetTime() == sampleTime)
				{
					return m_keyframes[index].GetValue();
				}
				if (m_keyframes[index].GetTime() > sampleTime)
				{
					const keyframeType& firstKeyframe(m_keyframes[index - 1]);
					const keyframeType& secondKeyframe(m_keyframes[index]);
					float ratio((sampleTime - firstKeyframe.GetTime())/(secondKeyframe.GetTime() - firstKeyframe.GetTime()));
					return firstKeyframe.GetValue() + ratio * (secondKeyframe.GetValue() - firstKeyframe.GetValue());
				}
			}
			return m_keyframes[m_keyframes.size() - 1].GetValue();
		}
	}
public:
	AttributeAnimation& operator=(const AttributeAnimation& other)
	{
		m_keyframes = other.m_keyframes;
		return *this;
	}

	AttributeAnimation& operator=(AttributeAnimation&& other) noexcept
	{
		m_keyframes = std::move(other.m_keyframes);
		return *this;
	}
private:
	keyframeContainerType m_keyframes;
};

// Attributos podem possuir diversos componenetes (e.g., o attributo posição do componente Transform possui 3 componentes: x, y e z).
// Essa classe é usada para armazenar animações de componentes de atributos de um certo componente.
// Perceba que a palavra componente pode apresentar duas semânticas distintas. Uma é usada para se referir a componentes de entidades (e.g., Transform),
// enquanto outra para componentes de atributos (componente x de posição).
//
// Attributes can have a set of components (e.g., the attribute position of component Transform have 3 components: x, y, and z).
// This class is used to store animations of components of attributes of a given component. Note that the word component have two distint meanings. One is
// used to refer to entity components (e.g., Transform) and the other to attribute components (e.g., component x of position).
template <class KeyframeValueType>
class AttributeAnimations
{
public:
	using keyframeValueType = KeyframeValueType;
	using keyframeType = Keyframe<keyframeValueType>;
	using attributeAnimationContainerType = std::vector<AttributeAnimation<keyframeValueType>>;
public:
	AttributeAnimations(size_t numberOfComponents)
	{
		m_animations.resize(numberOfComponents);
	}
	AttributeAnimations(const AttributeAnimations& other)
		:
		m_animations(other.m_animations)
	{}
	AttributeAnimations(AttributeAnimations&& other) noexcept
		:
		m_animations(std::move(other.m_animations))
	{}
	~AttributeAnimations() = default;
public:
	void AddKeyframe(size_t attributeComponentIndex, const keyframeType& keyframe)
	{
		DASSERT_E(attributeComponentIndex < m_animations.size());
		m_animations[attributeComponentIndex].AddKeyframe(keyframe);
	}

	keyframeValueType Sample(size_t attributeComponentIndex, float sampleTime) const
	{
		DASSERT_E(attributeComponentIndex < m_animations.size());
		return m_animations[attributeComponentIndex].Sample(sampleTime);
	}
public:
	AttributeAnimations& operator=(const AttributeAnimations& other)
	{
		m_animations = other.m_animations;
		return *this;
	}

	AttributeAnimations& operator=(AttributeAnimations&& other) noexcept
	{
		m_animations = std::move(other.m_animations);
		return *this;
	}
private:
	attributeAnimationContainerType m_animations;
};

class ComponentAnimations
{
public:
	using attributeIndexContainerType = SparseSet<AttributeIdType>;
	using integerAnimationsContainerType = std::vector<AttributeAnimations<int>>;
	using floatAnimationsContainerType = std::vector<AttributeAnimations<float>>;
public:
	ComponentAnimations() = default;
	ComponentAnimations(ComponentAnimations&&) noexcept;
	~ComponentAnimations() = default;
public:
	ComponentAnimations& operator=(ComponentAnimations&&) noexcept;
public:
	template <class KeyframeValueType>
	void MakeAttributeAnimation(AttributeIdType attributeIndex, size_t numberOfComponents)
	{
		static_assert(std::is_same_v<int, KeyframeValueType> || std::is_same_v<float, KeyframeValueType>, "Invalid keyframe value type.");
		const auto makeAttributeAnimation
		(
			[&](auto& animations) -> void
			{
				using attributeAnimationsType = typename std::remove_reference_t<decltype(animations)>::value_type;
				if (attributeIndex >= animations.size())
				{
					animations.resize(attributeIndex + 1, attributeAnimationsType(numberOfComponents));
					return;
				}
				animations[attributeIndex].~attributeAnimationsType();
				new (&animations[attributeIndex]) attributeAnimationsType(numberOfComponents);
			}
		);
		DASSERT_E(!m_attributeIndexes.Exists(attributeIndex));
		m_attributeIndexes.Add(attributeIndex);
		if constexpr (std::is_same_v<int, KeyframeValueType>)
		{
			makeAttributeAnimation(m_integerAnimations);
		}
		else
		{
			makeAttributeAnimation(m_floatAnimations);
		}
	}

	template <class KeyframeValueType>
	void AddKeyframe(AttributeIdType attributeIndex, size_t attributeComponentIndex, const Keyframe<KeyframeValueType>& keyframe)
	{
		static_assert(std::is_same_v<int, KeyframeValueType> || std::is_same_v<float, KeyframeValueType>, "Invalid keyframe value type.");
		auto addKeyframe
		(
			[&](auto& animations) -> void
			{
				animations[attributeIndex].AddKeyframe(attributeComponentIndex, keyframe);
			}
		);
		DASSERT_E(m_attributeIndexes.Exists(attributeIndex));
		if constexpr (std::is_same_v<int, KeyframeValueType>)
		{
			addKeyframe(m_integerAnimations);
		}
		else
		{
			addKeyframe(m_floatAnimations);
		}
	}

	template <class KeyframeValueType>
	KeyframeValueType Sample(AttributeIdType attributeIndex, size_t attributeComponentIndex, float sampleTime) const
	{
		static_assert(std::is_same_v<int, KeyframeValueType> || std::is_same_v<float, KeyframeValueType>, "Invalid keyframe value type.");
		DASSERT_E(m_attributeIndexes.Exists(attributeIndex));
		if constexpr (std::is_same_v<int, KeyframeValueType>)
		{
			return m_integerAnimations[attributeIndex].Sample(attributeComponentIndex, sampleTime);
		}
		else
		{
			return m_floatAnimations[attributeIndex].Sample(attributeComponentIndex, sampleTime);
		}
	}

	template <class Function>
	void IterateThroughAttributes(Function func)
	{
		for (AttributeIdType attributeId : m_attributeIndexes.GetDenseRef())
		{
			if (std::invoke(func, attributeId))
			{
				return;
			}
		}
	}
private:
	integerAnimationsContainerType m_integerAnimations;
	floatAnimationsContainerType m_floatAnimations;
	attributeIndexContainerType m_attributeIndexes;
};

class Event
{
public:
	Event()
		:
		m_id(0),
		m_time(0.0f)
	{}
	Event(Event&& other) noexcept
		:
		m_id(other.m_id),
		m_time(other.m_time)
	{}
	~Event() = default;
public:
	void SetId(size_t value)
	{
		m_id = value;
	}

	void SetTime(float time)
	{
		DASSERT_E(time >= 0.0f);
		m_time = time;
	}

	size_t GetId() const
	{
		return m_id;
	}

	float GetTime() const
	{
		return m_time;
	}
public:
	Event& operator=(const Event& other)
	{
		m_id = other.m_id;
		m_time = other.m_time;
		return *this;
	}
private:
	size_t m_id;
	float m_time;
};

class Animation
{
public:
	using componentIndexContainerType = SparseSet<ComponentIdType>;
	using metachannelContainerType = ReciclingVector<Event>;
	using stringType = std::string;
	using componentAnimationsContainerType = std::vector<ComponentAnimations>;
public:
	static constexpr float sampleRate{30.0f};
public:
	Animation(const stringType& name, float duration);
	Animation(Animation&&) noexcept;
	~Animation() = default;
public:
	void MakeAnimation(ComponentIdType componentIndex, AttributeIdType attributeIndex);
	metachannelContainerType::Ref MakeMetachannel();
	void RemoveMetachannelAtIndex(size_t index);
public:
	Animation& operator=(Animation&&) noexcept;
public:
	const stringType& GetName() const
	{
		return m_name;
	}

	void SetName(const stringType& name)
	{
		m_name = name;
	}

	float GetDuration() const
	{
		return m_duration;
	}
public:
	template <class KeyframeValueType>
	void AddKeyframe(ComponentIdType componentIndex, AttributeIdType attributeIndex, size_t attributeComponentIndex, const Keyframe<KeyframeValueType>& keyframe)
	{
		DASSERT_E(ComponentForms::Get().ExistsComponentWithId(componentIndex));
		DASSERT_E(ComponentForms::Get().ExistsComponentWithAttributeId(componentIndex, attributeIndex));
		DASSERT_E(m_componentIndexes.Exists(componentIndex));
		const ComponentForm& componentForm(ComponentForms::Get()[componentIndex]);
		const SerializedAttribute& attribute(componentForm.SerializedAttributes[attributeIndex]);
		DASSERT_E(attributeComponentIndex < attribute.GetNumberOfAttributeComponents());
		const AttributeKeyframeType keyframeType(attribute.GetKeyframeType());
		DASSERT_E(attribute.GetKeyframeType() != AttributeKeyframeType::NotDefined);
		DASSERT_E((keyframeType == AttributeKeyframeType::Integer && std::is_same_v<int, KeyframeValueType>) ||
					(keyframeType == AttributeKeyframeType::Float && std::is_same_v<float, KeyframeValueType>));
		m_componentAnimations[componentIndex].AddKeyframe(attributeIndex, attributeComponentIndex, keyframe);
	}
	
	template <class KeyframeValueType>
	KeyframeValueType Sample(ComponentIdType componentIndex, AttributeIdType attributeIndex, size_t attributeComponentIndex, float sampleTime) const
	{
		DASSERT_E(ComponentForms::Get().ExistsComponentWithId(componentIndex));
		DASSERT_E(ComponentForms::Get().ExistsComponentWithAttributeId(componentIndex, attributeIndex));
		DASSERT_E(m_componentIndexes.Exists(componentIndex));
		const ComponentForm& componentForm(ComponentForms::Get()[componentIndex]);
		const SerializedAttribute& attribute(componentForm.SerializedAttributes[attributeIndex]);
		DASSERT_E(attributeComponentIndex < attribute.GetNumberOfAttributeComponents());
		const AttributeKeyframeType keyframeType(attribute.GetKeyframeType());
		DASSERT_E(attribute.GetKeyframeType() != AttributeKeyframeType::NotDefined);
		DASSERT_E((keyframeType == AttributeKeyframeType::Integer && std::is_same_v<int, KeyframeValueType>) ||
					(keyframeType == AttributeKeyframeType::Float && std::is_same_v<float, KeyframeValueType>));
		return m_componentAnimations[componentIndex].Sample<KeyframeValueType>(attributeIndex, attributeComponentIndex, sampleTime);
	}

	template <class Function>
	void IterateThroughComponents(Function func)
	{
		for (ComponentIdType componentId : m_componentIndexes.GetDenseRef())
		{
			if (std::invoke(func, componentId))
			{
				return;
			}
		}
	}

	template <class Function>
	void IterateThroughAttributes(ComponentIdType componentIndex, Function func)
	{
		DASSERT_E(m_componentIndexes.Exists(componentIndex));
		m_componentAnimations[componentIndex].IterateThroughAttributes(func);
	}

	template <class Function>
	void IterateOnMetachannels(Function func)
	{
		m_metachannels.Iterate
		(
			[&](metachannelContainerType::Ref metachannel) -> bool
			{
				return std::invoke(func, metachannel);
			}
		);
	}

	template <class Function>
	void TryGetMetachannelsIds(float initialTime, float finalTime, Function func) const
	{
		m_metachannels.Iterate
		(
			[&](metachannelContainerType::ConstRef metachannel) -> bool
			{
				const float metachannelTime(metachannel->GetTime());
				if (metachannelTime == 0.0f && initialTime == 0.0f)
				{
					std::invoke(func, metachannel->GetId());
				}
				else if (metachannelTime > initialTime && metachannelTime <= finalTime)
				{
					std::invoke(func, metachannel->GetId());
				}
				return false;
			}
		);
	}
private:
	stringType m_name;
	float m_duration;
	componentAnimationsContainerType m_componentAnimations;
	componentIndexContainerType m_componentIndexes;
	metachannelContainerType m_metachannels;
};

using InternalAnimationRefType = typename AssetContainerType<Animation>::Ref;

class AnimationRef
{
public:
	using stringType = std::string;
public:
	AnimationRef();
	AnimationRef(InternalAnimationRefType, LockData&);
	AnimationRef(const AnimationRef&);
	~AnimationRef() = default;
public:
	bool IsValid() const;
	stringType GetName() const;
	float GetDuration() const;
	UUIDType GetUUID() const;
	void Invalidate();
	void Unload();
public:
	AnimationRef& operator=(Animation&&) noexcept;
public:
	AnimationRef& operator=(const AnimationRef& other)
	{
		m_ref = other.m_ref;
		m_lockData = other.m_lockData;
		return *this;
	}
public:
	template <class KeyframeValueType>
	KeyframeValueType Sample(ComponentIdType componentIndex, AttributeIdType attributeIndex, size_t attributeComponentIndex, float sampleTime) const
	{
		DASSERT_E(IsValid());
		return m_ref->GetAsset().Sample<KeyframeValueType>(componentIndex, attributeIndex, attributeComponentIndex, sampleTime);
	}

	template <class Function>
	void IterateThroughComponents(Function func)
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().IterateThroughComponents(func);
	}

	template <class Function>
	void IterateThroughAttributes(ComponentIdType componentIndex, Function func)
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().IterateThroughAttributes(componentIndex, func);
	}

	template <class Function>
	void TryGetMetachannelsIds(float initialTime, float finalTime, Function metachannelsTime)
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().TryGetMetachannelsIds(initialTime, finalTime, metachannelsTime);
	}
private:
	InternalAnimationRefType m_ref;
	LockData* m_lockData;
};

}
