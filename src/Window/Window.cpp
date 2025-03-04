#include "Window.h"



namespace DEditor
{

void Window::DispatchDragAndDropEvent(size_t count, const char** paths)
{
	for (const dragAndDropCallbackType& callback : m_dragAndDropCallbacks)
	{
		callback(count, paths);
	}
}

void Window::AddDragAndDropCallback(const dragAndDropCallbackType& callback)
{
	m_dragAndDropCallbacks.push_back(callback);
}

}
