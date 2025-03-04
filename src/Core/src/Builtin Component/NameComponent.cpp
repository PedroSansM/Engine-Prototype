#include "NameComponent.h"
#include "TemplateUtils.h"

#include <cstring>



namespace DCore
{

NameComponentFormGenerator NameComponentFormGenerator::s_generator;

NameComponent::NameComponent(const ConstructorArgs<NameComponent>& args)
	:
	m_name(args.Name)
{}

}
