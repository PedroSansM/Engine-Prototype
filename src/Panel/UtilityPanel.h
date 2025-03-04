#pragma once

#include "EditorGameViewPanel.h"



namespace DEditor
{

class UtilityPanel
{
	friend class Panels;
public:
	~UtilityPanel() = default;
private:
	static UtilityPanel& Get()
	{
		static UtilityPanel utilityPanel;
		return utilityPanel;
	}
private:
	UtilityPanel() = default;
private:
	void Render();
};

}
