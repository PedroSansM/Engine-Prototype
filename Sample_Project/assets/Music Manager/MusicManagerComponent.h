#pragma once

#include "DommusCore.h"



namespace Game
{
	class MusicManagerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::MusicManagerComponent>
{
	ConstructorArgs()
	{}

	DCore::DSoundEventInstance MainMenuMusic;
	DCore::DSoundEventInstance BossMusic;
};
#pragma pack(pop)

namespace Game
{

class MusicManagerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_mainMenuSound{0};
	static constexpr size_t a_bossSound{1};
public:
	MusicManagerComponent(const DCore::ConstructorArgs<MusicManagerComponent>&);
	~MusicManagerComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
public:
	void PlayMainMenuMusic();
	void StopMainMenuMusic();
	void PlayBossMusic();
	void StopBossMusic();
private:
	DCore::DSoundEventInstance m_mainMenuSound;
	DCore::DSoundEventInstance m_bossSound;
};

class MusicManagerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~MusicManagerComponentScriptComponentFormGenerator() = default;
private:
	MusicManagerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"MusicManagerComponent",
			DCore::ComponentId::GetId<MusicManagerComponent>(),
			sizeof(MusicManagerComponent),
			sizeof(DCore::ConstructorArgs<MusicManagerComponent>),
			{{DCore::AttributeName("Main Menu Music"), DCore::AttributeType::SoundEventInstance, MusicManagerComponent::a_mainMenuSound},
			{DCore::AttributeName("Boss Music"), DCore::AttributeType::SoundEventInstance, MusicManagerComponent::a_bossSound}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) MusicManagerComponent(*static_cast<const DCore::ConstructorArgs<MusicManagerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<MusicManagerComponent*>(componentAddress)->~MusicManagerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<MusicManagerComponent> m_defaultArgs;
private:
	static MusicManagerComponentScriptComponentFormGenerator s_generator;
};

}
	