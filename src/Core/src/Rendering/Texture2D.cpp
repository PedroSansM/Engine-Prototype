#include "Texture2D.h"
#include "DCoreAssert.h"
#include "AssetManager.h"
#include "ReadWriteLockGuard.h"
#include "Asset.h"

#include <cassert>



namespace DCore 
{

// Texture 2D Metadata
Texture2DMetadata::Texture2DMetadata()
	:
	m_srgb(false),
	m_alphaMask(false),
	m_filter(Texture2DFilter::Bilinear)
{}

Texture2DMetadata::Texture2DMetadata(bool srgb, bool alphaMask, Texture2DFilter filter)
	:
	m_srgb(srgb),
	m_alphaMask(alphaMask),
	m_filter(filter)
{}

Texture2DFilter Texture2DMetadata::StringToFilter(const DString& string)
{
	if (string == "Nearest")
	{
		return Texture2DFilter::Nearest;
	}
	else if (string == "Bilinear")
	{
		return Texture2DFilter::Bilinear;
	}
	else if (string == "Trilinear")
	{
		return Texture2DFilter::Trilinear;
	}
	DASSERT_E(false);
	return Texture2DFilter::Default;
}

const char* Texture2DMetadata::FilterToString(Texture2DFilter filter)
{
	switch (filter)
	{
	case Texture2DFilter::Nearest:
		return "Nearest";
	case Texture2DFilter::Bilinear:
		return "Bilinear";
	case Texture2DFilter::Trilinear:
		return "Trilinear";
	default:
		DASSERT_E(false);
		return "";
	}
}
// End Texture 2D Metadata

// Texture 2D
Texture2D::Texture2D()
	:
	m_valid(false)
{}

Texture2D::Texture2D(unsigned int id, const DVec2& size, int numberChannels, Texture2DMetadata metadata)
	:
	m_id(id),
	m_size(size),
	m_numberChannels(numberChannels),
	m_metadata(metadata),
	m_valid(true)
{}

Texture2D::Texture2D(Texture2D&& other) noexcept
	:
	m_id(other.m_id),
	m_size(other.m_size),
	m_numberChannels(other.m_numberChannels),
	m_metadata(other.m_metadata),
	m_valid(other.m_valid)
{
	other.m_valid = false;
}

Texture2D::~Texture2D()
{
	if (m_valid)
	{
		glDeleteTextures(1, &m_id);
	}
	m_valid = false;
}

void Texture2D::SetFilter(Texture2DFilter filter)
{
	m_metadata.SetFilterMethod(filter);
	glBindTexture(GL_TEXTURE_2D, m_id);
	switch (m_metadata.GetFilterMethod())
	{
	case Texture2DFilter::Nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		return;
	case Texture2DFilter::Bilinear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		return;
	case Texture2DFilter::Trilinear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		return;
	default:
		DASSERT_E(false);
		return;
	}
}
// End Texture 2D

// Begin Texture2DRef
Texture2DRef::Texture2DRef(InternalTexture2DRefType ref)
	:
	m_ref(ref)
{}

Texture2DRef::Texture2DRef(const Texture2DRef& other)
	:
	m_ref(other.m_ref)
{}

//exture2DRef::Texture2DRef(Texture2DRef&& other) noexcept
//:
//m_ref(other.m_ref),
//m_lockData(other.m_lockData)
//
//other.m_ref.Invalidate();
//

bool Texture2DRef::IsValid() const
{
	return m_ref.IsValid();
}

UUIDType Texture2DRef::GetUUID() const
{
	DASSERT_E(IsValid());
	return m_ref->GetUUID();
}

DVec2 Texture2DRef::GetDimensions() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetDimensions();
}

int Texture2DRef::GetNumberOfChannels() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetNumberOfChannels();
}

unsigned int Texture2DRef::GetId() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetId();
}

Texture2DFilter Texture2DRef::GetFilter() const
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetFilter();
}

void Texture2DRef::SetFilter(Texture2DFilter filter)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DASSERT_E(IsValid());
	m_ref->GetAsset().SetFilter(filter);
}

void Texture2DRef::Unload()
{
	ReadWriteLockGuard guard(LockType::WriteLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	if (m_ref.IsValid())
	{
		DCore::AssetManager::Get().UnloadTexture2D(m_ref->GetUUID());
	}
}

void Texture2DRef::Invalidate()
{
	m_ref.Invalidate();
}
// End Texture2DRef

}
