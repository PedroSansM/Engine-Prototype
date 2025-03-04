#pragma once

#include <cstddef>
#include <functional>
#include <vector>



namespace DEditor
{

class Window
{
public:
	using dragAndDropCallbackType = std::function<void(size_t, const char**)>;
	using dragAndDropCallbackContainerType = std::vector<dragAndDropCallbackType>;
public:
	Window(const Window&) = delete;
	Window(Window&&) = delete;
	~Window() = default;
public:
	static Window& Get()
	{
		static Window instance;
		return instance;
	}
public:
	void DispatchDragAndDropEvent(size_t count, const char** paths);
	void AddDragAndDropCallback(const dragAndDropCallbackType&);
private:
	Window() = default;
private:
	dragAndDropCallbackContainerType m_dragAndDropCallbacks;
};

}
