#pragma once

#include "SerializationTypes.h"
#include "Asset.h"
#include "Texture2D.h"
#include "UUID.h"
#include "ReadWriteLockGuard.h"

#include <atomic>



namespace DCore
{
enum class SpriteMaterialType
{	
	Default,
	Unlit,
	Lit
};

class SpriteMaterial
{
public:
	using stringType = std::string;
public:
	SpriteMaterial(SpriteMaterialType);
	SpriteMaterial(SpriteMaterial&&) noexcept;
	~SpriteMaterial() = default;
public:
	static stringType SpriteMaterialTypeToString(SpriteMaterialType);
	static SpriteMaterialType StringToSpriteMaterialType(const stringType&);
public:
	void SetAmbientMapRef(Texture2DRef ambientMapRef)
	{
		m_ambientMapRef = ambientMapRef;
	}

	void SetDiffuseMapRef(Texture2DRef diffuseMapRef)
	{
		m_diffuseMapRef = diffuseMapRef;
	}

	void SetSpecularMapRef(Texture2DRef specularMapRef)
	{
		m_specularMapRef = specularMapRef;
	}

	void SetGlossiness(DCore::DFloat glossiness)
	{
		m_glossiness = glossiness;
	}

	DVec4 GetDiffuseColor() const
	{
		return m_diffuseColor;
	}

	SpriteMaterialType GetType() const
	{
		return m_type;
	}

	Texture2DRef GetAmbientMapRef()
	{
		return m_ambientMapRef;
	}

	Texture2DRef GetDiffuseMapRef()
	{
		return m_diffuseMapRef;
	}

	Texture2DRef GetSpecularMapRef()
	{
		return m_specularMapRef;
	}

	const Texture2DRef GetAmbientMapRef() const
	{
		return m_ambientMapRef;
	}

	const Texture2DRef GetDiffuseMapRef() const
	{
		return m_diffuseMapRef;
	}

	const Texture2DRef GetSpecularMapRef() const
	{
		return m_specularMapRef;
	}

	float GetGlossiness() const
	{
		return m_glossiness;
	}

	void SetDiffuseColor(DVec4 diffuseColor)
	{
		m_diffuseColor = diffuseColor;
	}

	void ClearDiffuseMapRef()
	{
		m_diffuseMapRef.Invalidate();
	}
#ifdef EDITOR
	const DString& GetName() const
	{
		return m_name;
	}

	void SetName(const DString& name)
	{
		m_name = name;
	}
#endif
private:
	SpriteMaterialType m_type;
	Texture2DRef m_ambientMapRef;
	Texture2DRef m_diffuseMapRef;
	Texture2DRef m_specularMapRef;
	float m_glossiness;
	DVec4 m_diffuseColor;
#ifdef EDITOR
	DString m_name;
#endif
};

using InternalSpriteMaterialRefType = typename AssetContainerType<SpriteMaterial>::Ref;

class SpriteMaterialRef
{
public:
	SpriteMaterialRef();
	SpriteMaterialRef(InternalSpriteMaterialRefType ref, LockData& lockData);
	SpriteMaterialRef(const SpriteMaterialRef&);
	SpriteMaterialRef(SpriteMaterialRef&& other) noexcept;
	~SpriteMaterialRef() = default;
public:
	bool IsValid() const;
	UUIDType GetUUID() const;
	void SetAmbientMapRef(Texture2DRef);
	void SetDiffuseMapRef(Texture2DRef);
	void SetSpecularMapRef(Texture2DRef);
	void SetGlossiness(DFloat);
	void SetDiffuseColor(const DVec4&);
	Texture2DRef GetAmbientMapRef();
	Texture2DRef GetDiffuseMapRef();
	Texture2DRef GetSpecularMapRef();
	const Texture2DRef GetAmbientMapRef() const;
	const Texture2DRef GetDiffuseMapRef() const;
	const Texture2DRef GetSpecularMapRef() const;
	DFloat GetGlossiness() const;
	DVec4 GetDiffuseColor() const;
	SpriteMaterialType GetType() const;
	void ClearDiffuseMapRef();
	void Clean();
	void Unload();
#ifdef EDITOR
	const DString& GetName();
	void SetName(const DString& name);
#endif
public:
	SpriteMaterialRef& operator=(const SpriteMaterialRef other)
	{
		m_ref = other.m_ref;
		m_lockData = other.m_lockData;
		return *this;
	}

	bool operator==(const SpriteMaterialRef other)
	{
		DASSERT_E(IsValid());
		return GetUUID() == other.GetUUID();
	}
private:
	InternalSpriteMaterialRefType m_ref;
	LockData* m_lockData;
};

}
