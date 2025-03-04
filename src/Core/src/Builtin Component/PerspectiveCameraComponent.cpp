#include "PerspectiveCameraComponent.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"



namespace DCore
{

PerspectiveCameraComponent::PerspectiveCameraComponent(ConstructorArgs<PerspectiveCameraComponent>& args)
	:
	m_fov(args.FOV),
	m_near(args.Near),
	m_far(args.Far)
{}

PerspectiveCameraComponent::PerspectiveCameraComponent(ConstructorArgs<PerspectiveCameraComponent>&& args)
	:
	m_fov(args.FOV),
	m_near(args.Near),
	m_far(args.Far)
{}

void PerspectiveCameraComponent::OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_fov:
		m_fov = *static_cast<DUInt*>(newValue);
		m_fov = (std::max)((std::min)(static_cast<DUInt>(90), m_fov), static_cast<DUInt>(1));		
		break;
	case a_near:
		m_near = *static_cast<DFloat*>(newValue);
		m_near = (std::min)((std::max)(0.1f, m_near), m_far - 0.1f);
		break;
	case a_far:
		m_far = *static_cast<DFloat*>(newValue);
		m_far = (std::max)(m_near + 0.1f, m_far);
		break;
	}
}

DMat4 PerspectiveCameraComponent::GetProjectionMatrix(const DVec2& viewportSizes) const
{
	return glm::perspective(glm::radians((DCore::DFloat)m_fov), viewportSizes.x / viewportSizes.y, m_near, m_far);
}

PerspectiveCameraComponentFormGenerator PerspectiveCameraComponentFormGenerator::s_generator;

}
