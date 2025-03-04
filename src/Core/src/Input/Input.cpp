#include "Input.h"
#include "ReadWriteLockGuard.h"
#include "Runtime.h"



void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_RELEASE)
	{
		DCore::Input::Get().SetKeyState(key, action == GLFW_PRESS);
	}
	dUserKeyCallbackType userCallback(DCore::Input::Get().GetUserCallback());
	if (userCallback != nullptr)
	{
		(*userCallback)(window, key, scancode, action, mods);
	}
}

namespace DCore
{

Input::Input()
	:
	m_started(false),
	m_userCallback(nullptr)
{}

void Input::Start(GLFWwindow* context)
{
	if (m_started)
	{
		return;
	}
	m_started = true;
	glfwSetKeyCallback(context, KeyCallback);
}

void Input::SetKeyState(int key, bool state)
{
	m_runtimes.Iterate(
		[&](decltype(m_runtimes)::Ref runtimeRef) -> bool
		{
			Runtime* runtime(*runtimeRef.Data());
			runtime->AddKeyEvent({ key, state ? KeyEventType::Pressed : KeyEventType::Released });
			return false;
		});
}

void Input::SetUserCallback(dUserKeyCallbackType userCallback)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	m_userCallback = userCallback;
}

dUserKeyCallbackType Input::GetUserCallback()
{
	ReadWriteLockGuard guard(LockType::ReadLock, m_lockData);
	return m_userCallback;
}

size_t Input::AddRuntime(Runtime* runtime)
{
	auto ref(m_runtimes.PushBack(runtime));
	return ref.GetIndex();
}

void Input::RemoveRuntime(size_t runtimeIndex)
{
	m_runtimes.RemoveElementAtIndex(runtimeIndex);
}

}
