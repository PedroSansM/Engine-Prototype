#pragma once

#include "EntityRef.h"
#include "Animation.h"

#include <iostream>



namespace DCore
{

class AnimationSimulator
{
public:
	AnimationSimulator(const AnimationSimulator&) = delete;
	AnimationSimulator(AnimationSimulator&&) = delete;
	~AnimationSimulator() = default;
public:
	template <class Func>
	static void Simulate(EntityRef entity, AnimationRef animation, float currentSampleTime, float nextSampleTime, Func metachannelsCallback)
	{
		if (!animation.IsValid() || !entity.IsValid())
		{
			return;
		}
		animation.IterateThroughComponents(
			[&](ComponentIdType componentId) -> bool
			{
				ComponentRef<Component> component(entity.GetComponent(componentId));
				const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
				if (!component.IsValid())
				{
					return false;
				}
				animation.IterateThroughAttributes
				(
					componentId, 
					[&](AttributeIdType attributeId) -> bool
					{
						const SerializedAttribute& attribute(componentForm.SerializedAttributes[attributeId]);
						int integerValues[128];
						float floatValues[128];
						for (size_t attributeComponentId(0); attributeComponentId < attribute.GetNumberOfAttributeComponents(); attributeComponentId++)
						{
							switch (attribute.GetKeyframeType())
							{
							case AttributeKeyframeType::Integer:
							{
								integerValues[attributeComponentId] = animation.Sample<int>(componentId, attributeId, attributeComponentId, currentSampleTime);
								break;
							}
							case AttributeKeyframeType::Float:
							{
								floatValues[attributeComponentId] = animation.Sample<float>(componentId, attributeId, attributeComponentId, currentSampleTime);
								break;
							}
							default:
								DASSERT_E(false);
								break;
							}
						}
						if (attribute.GetKeyframeType() == AttributeKeyframeType::Integer)
						{
							component.OnAttributeChange(attributeId, integerValues, AttributeType::Integer);
							return false;
						}
						component.OnAttributeChange(attributeId, floatValues, AttributeType::Float);
						return false;
					}
				);
				return false;
			});
		animation.TryGetMetachannelsIds(
			currentSampleTime, nextSampleTime,
			[&](size_t metachannelId) -> void
			{
				std::invoke(metachannelsCallback, entity, metachannelId);
			});
	}
private:
	AnimationSimulator() = default;	
};

}
