#include "TransformComponent.h"
#include "DCoreMath.h"
#include "TemplateUtils.h"

#include "glm/ext/matrix_transform.hpp"



namespace DCore
{

TransformComponent::TransformComponent(const ConstructorArgs<TransformComponent>& args)
	:
	m_translation(args.Translation),
	m_rotation(args.Rotation),
	m_scale(args.Scale),
	m_modelMatrix(1.0f),
	m_isDirty(false)
{
	UpdateModelMatrix();
}

TransformComponent::TransformComponent(ConstructorArgs<TransformComponent>&& args)
	:
	m_translation(args.Translation),
	m_rotation(args.Rotation),
	m_scale(args.Scale),
	m_modelMatrix(1.0f),
	m_isDirty(false)
{
	UpdateModelMatrix();
}

void TransformComponent::OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_translation:
		SetTranslation(*(DVec3*)newValue);
		break;
	case a_rotation:
		SetRotation(*(DFloat*)newValue);		
		return;
	case a_scale:
		SetScale(*(DVec2*)newValue);
		return;
	default:
		DASSERT_E(false);
		return;
	}
}

DMat4 TransformComponent::GetInverseModelMatrix() const
{
	return glm::inverse(m_modelMatrix);
}

void TransformComponent::UpdateModelMatrix()
{
	m_modelMatrix = glm::translate(glm::mat4(1.0f), m_translation);
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation), {0.0f, 0.0f, 1.0f});
	m_modelMatrix = glm::scale(m_modelMatrix, {m_scale.x, m_scale.y, 1.0f});
	DVec2 translation, scale;
	float rotation;
	Math::Decompose(m_modelMatrix, translation, rotation, scale);
}

TransformComponentFormGenerator TransformComponentFormGenerator::s_generator;

}
