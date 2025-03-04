#pragma once



namespace DEditor
{

class Panels
{
public:
	~Panels() = default;
public:
	static Panels& Get()
	{
		static Panels panels;
		return panels;
	}
public:
	void RenderPanels();
private:
	Panels() = default;
};

}
