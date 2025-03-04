#include "ChildrenComponent.h"
#include "ChildComponent.h"
#include "ComponentRef.h"
#include "Scene.h"
#include "TemplateUtils.h"



namespace DCore
{

ChildrenComponent::ChildrenComponent(const ConstructorArgs<ChildrenComponent>& args)
	:
	m_firstChild(args.FirstChild),
	m_numberOfChildren(args.NumberOfChildren)
{}

// Supõe-se que o pai já foi adicionado.
// Essa função deve apenas organizar a lista duplamente encadeada de entidades.
void ChildrenComponent::AddChild(EntityRef entity)
{
	ChildComponent& newChildComponent(entity.GetComponents<ChildComponent>().GetRawComponent());		
	if (m_numberOfChildren == 0)
	{
		newChildComponent.RemoveNext();
		newChildComponent.RemovePrevious();
	}
	else
	{
		ChildComponent& firstChildComponent(m_firstChild.GetComponents<ChildComponent>().GetRawComponent());
		firstChildComponent.SetPrevious(entity);
		newChildComponent.SetNext(m_firstChild);
	}
	m_firstChild = entity;
	m_numberOfChildren++;
}

void ChildrenComponent::RemoveChild(EntityRef entityRef)
{
	if (entityRef == m_firstChild)
	{
		if (m_numberOfChildren > 1)
		{
			ChildComponent& childComponent(entityRef.GetComponents<ChildComponent>().GetRawComponent());
			EntityRef next(childComponent.GetNext());
			ChildComponent& nextChildComponent(next.GetComponents<ChildComponent>().GetRawComponent());
			nextChildComponent.RemovePrevious();
			m_firstChild = next;
		}
		else
		{
			m_firstChild.Invalidate();
		}
	}
	else
	{
		if (m_numberOfChildren > 1)
		{
			EntityRef toRemove(m_firstChild);
			size_t jumpCount(0);
			while (toRemove != entityRef)
			{
				if (jumpCount >= m_numberOfChildren - 1)
				{
					return;
				}
				ChildComponent& childComponent(toRemove.GetComponents<ChildComponent>().GetRawComponent());
				toRemove = childComponent.GetNext();
				jumpCount++;
			}
			ChildComponent& toRemoveChildComponent(toRemove.GetComponents<ChildComponent>().GetRawComponent());
			EntityRef toRemovePrevious(toRemoveChildComponent.GetPrevious());
			ChildComponent& toRemovePreviousChildComponent(toRemovePrevious.GetComponents<ChildComponent>().GetRawComponent());
			if (toRemoveChildComponent.HaveNext())
			{
				EntityRef toRemoveNext(toRemoveChildComponent.GetNext());
				ChildComponent& toRemoveNextChildComponent(toRemoveNext.GetComponents<ChildComponent>().GetRawComponent());
				toRemoveNextChildComponent.SetPrevious(toRemovePrevious);
				toRemovePreviousChildComponent.SetNext(toRemoveNext);
			}
			else
			{
				toRemovePreviousChildComponent.RemoveNext();
			}
		}
	}
	m_numberOfChildren--;
}

bool ChildrenComponent::HaveChild(const EntityRef entityRef) const
{
	if (m_numberOfChildren == 0)
	{
		return false;
	}
	if (m_firstChild == entityRef)
	{
		return true;
	}
	if (m_numberOfChildren == 1)
	{
		return false;
	}
	const ChildComponent& firstChildComponent(m_firstChild.GetComponents<ChildComponent>().GetRawComponent());
	EntityRef currentEntity(firstChildComponent.GetNext());
	for (size_t i(1); i < m_numberOfChildren; i++)
	{
		if (currentEntity == entityRef)
		{
			return true;
		}
		ChildComponent& currentChildComponent(currentEntity.GetComponents<ChildComponent>().GetRawComponent());
		currentEntity = currentChildComponent.GetNext();
	}
	return false;
}

std::vector<EntityRef> ChildrenComponent::GetChildren() const
{
	std::vector<EntityRef> children;
	IterateOnChildren
	(
		[&](EntityRef entityRef) -> bool
		{
			children.push_back(entityRef);
			return false;
		}
	);
	return children;
}

ChildrenComponentFormGenerator ChildrenComponentFormGenerator::s_generator;

}
