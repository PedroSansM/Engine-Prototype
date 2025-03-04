#include "PhysicsAPI.h"



namespace DCore::Physics
{

b2BodyType CoreBodyTypeToBox2dBodyType(DBodyType bodyType)
{
	switch (bodyType)
	{
	case DBodyType::Static:
		return b2BodyType::b2_staticBody;
	case DBodyType::Kinematic:
		return b2BodyType::b2_kinematicBody;
	case DBodyType::Dynamic:
		return b2BodyType::b2_dynamicBody;
	default:
		DASSERT_E(false);
		return b2BodyType::b2_staticBody;
	}
}

}
