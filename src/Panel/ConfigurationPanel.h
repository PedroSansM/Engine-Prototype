#pragma once

#include "Panel.h"

#include "DommusCore.h"

#include <cstdarg>



namespace DEditor
{

class ConfigurationPanel : public Panel
{
public:
	using uuidType = DCore::UUIDType;
	using stringType = std::string;
public:
	static constexpr size_t filterNameSize{256};
public:
	ConfigurationPanel(const ConfigurationPanel&) = delete;
	ConfigurationPanel(ConfigurationPanel&&) = delete;
	~ConfigurationPanel() = default;
public:
	static ConfigurationPanel& Get()
	{
		static ConfigurationPanel configurationPanel;
		return configurationPanel;
	}
public:
	void Render();
public:
	void Open()
	{
		m_isOpened = true;
	}
private:
	ConfigurationPanel();
private:
	stringType m_startingSceneName;
	bool m_isOpened;
	int m_windowFlags;
private:
	void SetPanelToUnsavedState();
};

}
