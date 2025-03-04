#include "BoxColliderComponent.h"

#include <string.h>



namespace DCore
{

BoxColliderComponent::BoxColliderComponent(const ConstructorArgs<BoxColliderComponent>& args)
	:
	m_bodyType(args.BodyType),
	m_enabled(args.Enabled),
	m_isSensor(args.IsSensor),
	m_gravityScale(args.GravityScale),
	m_fixedRotation(args.FixedRotation),
	m_useCCD(args.UseCCD),
	m_offset(args.Offset),
	m_sizes(args.Sizes),
	m_physicsMaterial(args.PhysicsMaterial),
	m_selfPhysicsLayer(args.SelfPhysicsLayer),
	m_collideWithPhysicsLayer(args.CollideWithPhysicsLayers),
	m_drawCollider(args.DrawCollider),
	m_bodyId(b2_nullBodyId),
	m_shapeId(b2_nullShapeId),
	m_dirtyType(0)
{}	

BoxColliderComponent::~BoxColliderComponent()
{
	m_physicsMaterial.Unload();
}

void* BoxColliderComponent::GetAttributePtr(AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_bodyType:
		return &m_bodyType;
	case a_enabled:
		return &m_enabled;
	case a_isSensor:
		return &m_isSensor;
	case a_gravityScale:
		return &m_gravityScale;
	case a_fixedRotation:
		return &m_fixedRotation;
	case a_useCCD:
		return &m_useCCD;
	case a_offset:
		return &m_offset;
	case a_sizes:
		return &m_sizes;
	case a_physicsMaterial:
		return &m_physicsMaterial;
	case a_selfPhysicsLayer:
		return &m_selfPhysicsLayer;
	case a_collideWithPhysicsLayers:
		return &m_collideWithPhysicsLayer;
	case a_drawCollider:
		return &m_drawCollider;
	default:
		return nullptr;
	}
}

void BoxColliderComponent::OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_bodyType:
	{
		SetBodyType(*static_cast<DBodyType*>(newValue));
		return;
	}
	case a_enabled:
	{
		SetEnabled(*static_cast<DLogic*>(newValue));
		return;
	}
	case a_isSensor:
	{
		SetIsSensor(*static_cast<DLogic*>(newValue));
		return;
	}
	case a_gravityScale:
	{
		SetGravityScale(*static_cast<DFloat*>(newValue));
		return;
	}
	case a_fixedRotation:
	{
		SetFixedRotation(*static_cast<DLogic*>(newValue));
		return;
	}
	case a_useCCD:
	{
		SetUseCCD(*static_cast<DLogic*>(newValue));
		return;
	}
	case a_offset:
	{
		SetOffset(*static_cast<DVec2*>(newValue));
		return;
	}
	case a_sizes:
	{
		const DVec2& sizes(*static_cast<DVec2*>(newValue));
		SetSizes(sizes);
		return;
	}
	case a_physicsMaterial:
	{
		m_physicsMaterial.Unload();
		SetPhysicsMaterial(*static_cast<PhysicsMaterialRef*>(newValue));
		return;
	}
	case a_selfPhysicsLayer:
	{
		SetSelfPhysicsLayer(*static_cast<DPhysicsLayer*>(newValue));
		return;
	}
	case a_collideWithPhysicsLayers:
	{
		SetColliderWithPhysicsLayers(*static_cast<DPhysicsLayer*>(newValue));
		return;
	}
	case a_drawCollider:
	{
		SetDrawCollider(*static_cast<DLogic*>(newValue));
		return;
	}
	default:
		return;
	}
}

BoxColliderComponentFormGenerator BoxColliderComponentFormGenerator::s_generator;

}
