#include "Sound.h"
#include "DCoreAssert.h"
#include "AssetManager.h"
#include "TransformComponent.h"
#include "AudioListenerComponent.h"

#include "fmod_studio.hpp"
#include "fmod_errors.h"

#include <vector>



namespace DCore
{

Sound::Sound()
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	FMOD_RESULT result(FMOD::Studio::System::create(&m_system));
	DASSERT_E(result == FMOD_OK);
	result = m_system->initialize(512, FMOD_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
	DASSERT_E(result == FMOD_OK);
}

void Sound::AddBank(const pathType& path, ReturnError& returnError)
{
	DASSERT_E(m_system != nullptr);
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	FMOD::Studio::Bank* newBank(nullptr);
	FMOD_RESULT result(m_system->loadBankFile(path.string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &newBank));
	if (result != FMOD_OK)
	{
		returnError.Ok = false;
		returnError.Message = FMOD_ErrorString(result);
		return;
	}
	returnError.Ok = true;
	m_loadedBanks.push_back(newBank);
}

Sound::eventInstanceType* Sound::TryCreateEventInstance(const char* path)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	FMOD::Studio::EventDescription* ed(nullptr);
	FMOD_RESULT result(m_system->getEvent(path, &ed));
	if (result != FMOD_OK)
	{
		return nullptr;
	}
	eventInstanceType* ei(nullptr);
	result = ed->createInstance(&ei);
	if (result != FMOD_OK)
	{
		return nullptr;
	}
	return ei;
}	

void Sound::UnloadAllBanks()
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	for (bankType* bank : m_loadedBanks)
	{
		bank->unload();
	}
	m_loadedBanks.clear();
}

void Sound::Update3DAudioListener()
{
	ReadWriteLockGuard guard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			bool audioListenerFound(false);
			scene.Iterate<TransformComponent, AudioListenerComponent>(
				[&](Entity entity, ComponentRef<TransformComponent> transformComponent, ComponentRef<AudioListenerComponent>) -> bool
				{
					FMOD_3D_ATTRIBUTES attributes;
					DVec3 translation(transformComponent.GetTranslation());
					attributes.position = { translation.x, translation.y, translation.z };
					attributes.velocity = { 0.0f, 0.0f, 0.0f };
					attributes.forward = { 0.0f, 0.0f, 1.0f };
					attributes.up = { 0.0f, 1.0f, 0.0f };
					FMOD_RESULT result(m_system->setListenerAttributes(0, &attributes));
					DASSERT_E(result == FMOD_OK);
					audioListenerFound = true;
					return true;
				});
			return audioListenerFound;
		});
}

void Sound::Update()
{
	FMOD_RESULT result(m_system->update());
	DASSERT_E(result == FMOD_OK);
}

}
