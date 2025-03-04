#pragma once

#include "AssetManagerTypes.h"
#include "ReadWriteLockGuard.h"
#include "UUID.h"

#include <string>



namespace DCore
{

class PhysicsMaterial
{
public:
	using stringType = std::string;
public:
	static constexpr float defaultDensity{1.0f};
	static constexpr float defaultFriction{0.0f};
	static constexpr float defaultRestitution{0.0f};
public:
	PhysicsMaterial();
	PhysicsMaterial(float density, float friction, float restitution);
	PhysicsMaterial(PhysicsMaterial&& other) noexcept;
	~PhysicsMaterial() = default;
public:
	float GetDensity() const
	{
		return m_density;
	}

	float GetFriction() const
	{
		return m_friction;
	}

	float GetRestitution() const
	{
		return m_restitution;
	}

	void SetDensity(float value)
	{
		m_density = value;
	}

	void SetFriction(float value)
	{
		m_friction = value;
	}

	void SetRestitution(float value)
	{
		m_restitution = value;
	} 
#ifdef EDITOR
	const stringType& GetName() const
	{
		return m_name;
	}

	void SetName(const stringType& value)
	{
		m_name = value;
	}
#endif
private:
	float m_density;
	float m_friction;
	float m_restitution;
#ifdef EDITOR
	stringType m_name;
#endif
};

using InternalPhysicsMaterialRefType = typename AssetContainerType<PhysicsMaterial>::Ref;

class PhysicsMaterialRef
{
public:
	using stringType = PhysicsMaterial::stringType;
public:
	static constexpr float defaultDensity{PhysicsMaterial::defaultDensity};
	static constexpr float defaultFriction{PhysicsMaterial::defaultFriction};
	static constexpr float defaultRestitution{PhysicsMaterial::defaultRestitution};
public:
	PhysicsMaterialRef();
	PhysicsMaterialRef(InternalPhysicsMaterialRefType, LockData&);
	PhysicsMaterialRef(const PhysicsMaterialRef&);
	PhysicsMaterialRef(PhysicsMaterialRef&&) noexcept;
	~PhysicsMaterialRef() = default;
public:
	bool IsValid() const;
	void Invalidate();
	UUIDType GetUUID() const;
	float GetDensity() const;
	float GetFriction() const;
	float GetRestitution() const;
	void SetDensity(float value);
	void SetFriction(float value);
	void SetRestitution(float value);
	void Unload();
#ifdef EDITOR
	stringType GetName() const;
	void SetName(const stringType&);
#endif
public:
	PhysicsMaterialRef& operator=(const PhysicsMaterialRef&);
private:
	InternalPhysicsMaterialRefType m_ref;
	LockData* m_lockData;
};

}
