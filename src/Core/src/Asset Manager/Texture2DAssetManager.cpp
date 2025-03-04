#include "Texture2DAssetManager.h"
#include "Graphics.h"



namespace DCore 
{

Texture2DAssetManager::~Texture2DAssetManager()
{
	m_textures.Clear();
	m_loadedTextures2D.clear();
}

bool Texture2DAssetManager::IsTexture2DLoaded(const UUIDType& uuid)
{
	ReadWriteLockGuard guard(LockType::ReadLock, m_lockData);
	return m_loadedTextures2D.find(uuid) != m_loadedTextures2D.end();
}

Texture2DRef Texture2DAssetManager::LoadTexture2D(const UUIDType& uuid, unsigned char* binary, const DVec2& size, int numberChannels, Texture2DMetadata metadata)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	DASSERT_E(m_loadedTextures2D.find(uuid) == m_loadedTextures2D.end());
	Texture2D texture2D(GenerateTexture2D(binary, size, numberChannels, metadata));
	InternalTexture2DRefType internalRef(m_textures.PushBack(uuid, std::move(texture2D)));
	m_loadedTextures2D.insert({uuid, internalRef});
	return Texture2DRef(internalRef, m_lockData);
}

Texture2D Texture2DAssetManager::LoadRawTexture2D(unsigned char* binary, const DVec2& size, int numberChannels, Texture2DMetadata metadata)
{
	return GenerateTexture2D(binary, size, numberChannels, metadata);
}

Texture2DRef Texture2DAssetManager::GetTexture2DRef(const UUIDType& uuid)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	DASSERT_E(m_loadedTextures2D.find(uuid) != m_loadedTextures2D.end());
	InternalTexture2DRefType internalRef(m_loadedTextures2D.find(uuid)->second);
	internalRef->AddReferenceCount();
	return Texture2DRef(internalRef, m_lockData);
}

void Texture2DAssetManager::UnloadTexture2D(const UUIDType& uuid, bool removeAllReferences)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	auto it(m_loadedTextures2D.find(uuid));
	if (it == m_loadedTextures2D.end())
	{
		return;
	}
	InternalTexture2DRefType internalRef(it->second);
	if (internalRef->m_referenceCount == 1 || removeAllReferences)
	{
		m_loadedTextures2D.erase(uuid);
		m_textures.Remove(internalRef);
		return;
	}
	internalRef->SubReferenceCount();
}

Texture2D Texture2DAssetManager::GenerateTexture2D(unsigned char* binary, const DVec2& size, int numberChannels, Texture2DMetadata metadata)
{
	unsigned int id(0);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	switch (metadata.GetFilterMethod()) 
	{
	case Texture2DFilter::Default:
		DASSERT_E(false);
		break;
	case Texture2DFilter::Nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case Texture2DFilter::Bilinear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		break;
	case Texture2DFilter::Trilinear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	switch (numberChannels)
	{
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size.x, size.y, 0, GL_RED, GL_UNSIGNED_BYTE, binary);
		break;
	case 2:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, size.x, size.y, 0, GL_RG, GL_UNSIGNED_BYTE, binary);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, binary);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, binary);
		break;
	default:
		assert(false);
		break;
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	return Texture2D(id, size, numberChannels, metadata);
}

}
