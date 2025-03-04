#include "ChildComponent.h"
#include "TemplateUtils.h"



namespace DCore
{

ChildComponent::ChildComponent(const ConstructorArgs<ChildComponent>& args)
	:
	m_parent(args.Parent),
	m_next(args.Next),
	m_previous(args.Previous)
{}

ChildComponentFormGenerator ChildComponentFormGenerator::s_generator;

}
