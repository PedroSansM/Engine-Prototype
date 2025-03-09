#include "PhysicsMaterial.h"
#include "Asset.h"
#include "ReciclingVector.h"
#include "AssetManager.h"



namespace DCore
{

// Physics Material
PhysicsMaterial::PhysicsMaterial()
	:
	m_density(defaultDensity),
	m_friction(defaultFriction),
	m_restitution(defaultRestitution)
{}

PhysicsMaterial::PhysicsMaterial(float density, float friction, float restitution)
	:
	m_density(density),
	m_friction(friction),
	m_restitution(restitution)
{}

PhysicsMaterial::PhysicsMaterial(PhysicsMaterial&& other) noexcept
	:
	m_density(other.m_density),
	m_friction(other.m_friction),
	m_restitution(other.m_restitution)
#ifdef EDITOR
	, m_name(std::move(other.m_name))
#endif
{}
// End Physics Material

// Physics Material Ref
PhysicsMaterialRef::PhysicsMaterialRef()
	:
	m_lockData(nullptr)
{}

PhysicsMaterialRef::PhysicsMaterialRef(InternalPhysicsMaterialRefType ref, LockData& lockData)
	:
	m_ref(ref),
	m_lockData(&lockData)
{}

PhysicsMaterialRef::PhysicsMaterialRef(const PhysicsMaterialRef& other)
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{}

PhysicsMaterialRef::PhysicsMaterialRef(PhysicsMaterialRef&& other) noexcept
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{
	other.m_ref.Invalidate();
}

bool PhysicsMaterialRef::IsValid() const
{
	return m_lockData != nullptr && m_ref.IsValid();
}

void PhysicsMaterialRef::Invalidate()	
{
	m_ref.Invalidate();
}

UUIDType PhysicsMaterialRef::GetUUID() const
{
	DASSERT_E(IsValid());
	return m_ref->GetUUID();
}

float PhysicsMaterialRef::GetDensity() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetDensity();
}

float PhysicsMaterialRef::GetFriction() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetFriction();
}

float PhysicsMaterialRef::GetRestitution() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetRestitution();
}

void PhysicsMaterialRef::SetDensity(float value)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetDensity(value);
}

void PhysicsMaterialRef::SetFriction(float value)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetFriction(value);
}

void PhysicsMaterialRef::SetRestitution(float value)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetRestitution(value);
}

void PhysicsMaterialRef::Unload()
{
	if (IsValid())
	{
		AssetManager::Get().UnloadPhysicsMaterial(m_ref->GetUUID());
	}
}

#ifdef EDITOR
PhysicsMaterialRef::stringType PhysicsMaterialRef::GetName() const
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::ReadLock, *m_lockData);
	return m_ref->GetAsset().GetName();
}

void PhysicsMaterialRef::SetName(const stringType& name)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetName(name);

}
#endif

PhysicsMaterialRef& PhysicsMaterialRef::operator=(const PhysicsMaterialRef& other)
{
	m_ref = other.m_ref;
	m_lockData = other.m_lockData;
	return *this;
}
// End Physics Material Ref

}
