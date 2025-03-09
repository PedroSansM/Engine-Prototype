#pragma once

#include "SerializationTypes.h"
#include "UUID.h"
#include "ReadWriteLockGuard.h"
#include "AssetManagerTypes.h"
#include "Graphics.h"

#include <cstddef>
#include <string>
#include <atomic>



namespace DCore
{

enum class Texture2DFilter : size_t
{
	Nearest = 0,
	Bilinear = 1,
	Trilinear = 2,
	Default = 3,
};

class Texture2DMetadata
{
public:
	Texture2DMetadata();
	Texture2DMetadata(bool srgb, bool alphaMask, Texture2DFilter filter);
	~Texture2DMetadata() = default;
public:
	static Texture2DFilter StringToFilter(const DString&);
	static const char* FilterToString(Texture2DFilter);
public:
	bool IsSRGB() const
	{
		return m_srgb;
	}

	bool IsAlphaMask() const
	{
		return m_alphaMask;
	}

	Texture2DFilter GetFilterMethod() const
	{
		return m_filter;
	}

	void SetFilterMethod(Texture2DFilter filter)
	{
		m_filter = filter;
	}
private:
	bool m_srgb;
	bool m_alphaMask;
	Texture2DFilter m_filter;
};

class Texture2D
{
public:
	Texture2D();
	Texture2D(unsigned int id, const DVec2& size, int numberChannels, Texture2DMetadata metadata);
	Texture2D(Texture2D&&) noexcept;
	~Texture2D();
public:
	void SetFilter(Texture2DFilter);
public:
	DVec2 GetDimensions() const
	{
		return m_size;
	}
	
	int GetNumberOfChannels() const
	{
		return m_numberChannels;
	}
	
	unsigned int GetId() const
	{
		return m_id;
	}

	bool IsValid() const
	{
		return m_valid;
	}

	Texture2DFilter GetFilter() const
	{
		return m_metadata.GetFilterMethod();
	}

	Texture2D& operator=(Texture2D&& other) noexcept
	{
		m_id = other.m_id;
		m_size = other.m_size;
		m_numberChannels = other.m_numberChannels;
		m_metadata = other.m_metadata;
		m_valid = other.m_valid;
		other.m_valid = false;
		return *this;
	}
private:
	unsigned int m_id;
	DVec2 m_size;
	int m_numberChannels;
	Texture2DMetadata m_metadata;
	bool m_valid;
};

using InternalTexture2DRefType = typename AssetContainerType<Texture2D>::Ref;

class Texture2DRef
{
public:
	Texture2DRef() = default;
	Texture2DRef(InternalTexture2DRefType ref);
	Texture2DRef(const Texture2DRef&);
	~Texture2DRef() = default;
public:
	bool IsValid() const;
	UUIDType GetUUID() const;
	DVec2 GetDimensions() const;
	int GetNumberOfChannels() const;
	unsigned int GetId() const;
	Texture2DFilter GetFilter() const;
	void SetFilter(Texture2DFilter);
	void Unload();
	void Invalidate();
public:
	InternalTexture2DRefType GetInternalRef()
	{
		return m_ref;
	}
public:
	Texture2DRef& operator=(const Texture2DRef& other)
	{
		m_ref = other.m_ref;
		return *this;
	}
private:
	InternalTexture2DRefType m_ref;
};

}
