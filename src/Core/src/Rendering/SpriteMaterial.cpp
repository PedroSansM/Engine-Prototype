#include "SpriteMaterial.h"
#include "DCoreAssert.h"
#include "AssetManager.h"
#include "AssetManagerTypes.h"
#include "ReadWriteLockGuard.h"
 
#include <cstring>



namespace DCore
{

// Begin SpriteMaterial
SpriteMaterial::SpriteMaterial(SpriteMaterialType type)
	:
	m_type(type),
	m_glossiness(0.5f),
	m_diffuseColor(1, 1, 1, 1)
{}

SpriteMaterial::SpriteMaterial(SpriteMaterial&& other) noexcept
	:
	m_type(other.m_type),
	m_ambientMapRef(other.m_ambientMapRef),
	m_diffuseMapRef(other.m_diffuseMapRef),
	m_specularMapRef(other.m_specularMapRef),
	m_glossiness(other.m_glossiness),
	m_diffuseColor(other.m_diffuseColor)
#ifdef EDITOR
	, m_name(std::move(other.m_name))
#endif
{
	other.m_diffuseMapRef.Invalidate();
	other.m_ambientMapRef.Invalidate();
	other.m_specularMapRef.Invalidate();
}

SpriteMaterial::stringType SpriteMaterial::SpriteMaterialTypeToString(SpriteMaterialType type)
{
	switch (type)
	{
	case SpriteMaterialType::Unlit:
		return "Unlit";
	case SpriteMaterialType::Lit:
		return "Lit";
	default:
		DASSERT_E(false);
		return "";
	}
}

SpriteMaterialType SpriteMaterial::StringToSpriteMaterialType(const stringType& typeString)
{
	if (typeString == "Unlit")
	{
		return SpriteMaterialType::Unlit;
	}
	else if (typeString == "Lit")
	{
		return SpriteMaterialType::Lit;
	}
	DASSERT_E(false);
	return SpriteMaterialType::Default;
}
// End SpriteMaterial

// Begin SpriteMaterialRef
SpriteMaterialRef::SpriteMaterialRef()
	:
	m_lockData(nullptr)
{}

SpriteMaterialRef::SpriteMaterialRef(InternalSpriteMaterialRefType ref, LockData& lockData)
	:
	m_ref(ref),
	m_lockData(&lockData)
{}

SpriteMaterialRef::SpriteMaterialRef(const SpriteMaterialRef& other)
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{}

SpriteMaterialRef::SpriteMaterialRef(SpriteMaterialRef&& other) noexcept
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{
	other.m_ref.Invalidate();
}

bool SpriteMaterialRef::IsValid() const
{
	return m_lockData != nullptr && m_ref.IsValid();
}

UUIDType SpriteMaterialRef::GetUUID() const
{
	DASSERT_E(IsValid());
	return m_ref->GetUUID();
}

void SpriteMaterialRef::SetAmbientMapRef(Texture2DRef texture2DRef)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetAmbientMapRef(texture2DRef);
}	

void SpriteMaterialRef::SetDiffuseMapRef(Texture2DRef texture2DRef)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetDiffuseMapRef(texture2DRef);
}

void SpriteMaterialRef::SetSpecularMapRef(Texture2DRef texture2DRef)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetSpecularMapRef(texture2DRef);
}

void SpriteMaterialRef::SetGlossiness(DFloat value)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetGlossiness(value);
}

void SpriteMaterialRef::SetDiffuseColor(const DVec4& color)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetDiffuseColor(color);
}

Texture2DRef SpriteMaterialRef::GetAmbientMapRef()
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetAmbientMapRef();
}

Texture2DRef SpriteMaterialRef::GetDiffuseMapRef()
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetDiffuseMapRef();
}

Texture2DRef SpriteMaterialRef::GetSpecularMapRef()
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetSpecularMapRef();
}

const Texture2DRef SpriteMaterialRef::GetAmbientMapRef() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetAmbientMapRef();
}

const Texture2DRef SpriteMaterialRef::GetDiffuseMapRef() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetDiffuseMapRef();
}

const Texture2DRef SpriteMaterialRef::GetSpecularMapRef() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetSpecularMapRef();
}

DFloat SpriteMaterialRef::GetGlossiness() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetGlossiness();
}

DVec4 SpriteMaterialRef::GetDiffuseColor() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetDiffuseColor();
}

SpriteMaterialType SpriteMaterialRef::GetType() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetType();
}

void SpriteMaterialRef::ClearDiffuseMapRef()
{
	DASSERT_E(IsValid());
	m_ref->GetAsset().ClearDiffuseMapRef();
}

void SpriteMaterialRef::Unload()
{
	if (m_ref.IsValid())
	{
		AssetManager::Get().UnloadSpriteMaterial(m_ref->GetUUID());
	}
}

#ifdef EDITOR
const DString& SpriteMaterialRef::GetName()
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetName();
}

void SpriteMaterialRef::SetName(const DString& name)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetName(name);
}
#endif
// End SpriteMaterialRef

}
