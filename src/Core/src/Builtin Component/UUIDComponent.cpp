#include "UUIDComponent.h"
#include "TemplateUtils.h"



namespace DCore
{

UUIDComponent::UUIDComponent(const ConstructorArgs<UUIDComponent>& args)
	:
	m_uuid(args.UUID)
{}

 UUIDComponentFormGenerator UUIDComponentFormGenerator::s_generator;

}
