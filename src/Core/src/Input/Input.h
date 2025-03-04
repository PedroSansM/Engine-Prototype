#pragma once

#include "ReadWriteLockGuard.h"
#include "ReciclingVector.h"

#include "Graphics.h"

#include <cstddef>
#include <array>



using dUserKeyCallbackType = void (*)(GLFWwindow* window, int key, int scancode, int action, int mods);

namespace DCore
{

#define KEY_TO_INT(Key) static_cast<int>(Key)
#define INT_TO_KEY(Int) static_cast<DKey>(Int)

class Runtime;

enum class DKey : int
{
	Space = GLFW_KEY_SPACE,
	W = GLFW_KEY_W,
	A = GLFW_KEY_A,
	S = GLFW_KEY_S,
	D = GLFW_KEY_D,
	Z = GLFW_KEY_Z,
	X = GLFW_KEY_X,
	C = GLFW_KEY_C,
	R = GLFW_KEY_R,
	ArrowDown = GLFW_KEY_DOWN,
	ArrowUp = GLFW_KEY_UP,
	ArrowRight = GLFW_KEY_RIGHT,
	ArrowLeft = GLFW_KEY_LEFT,
	L_Shift = GLFW_KEY_LEFT_SHIFT,
};

enum class KeyEventType
{
	Pressed,
	Released
};

typedef 
struct KeyEvent
{
	int Key;
	KeyEventType Event;
} KeyEvent;

class Input
{
	friend class ReadWriteLockGuard;
public:
	static constexpr size_t numberOfKeys{GLFW_KEY_LAST};
public:
	Input(const Input&) = delete;	
	Input(Input&&) = delete;
	~Input() = default;
public:
	static Input& Get()
	{
		static Input input;
		return input;
	}
public:
	void Start(GLFWwindow*);
	void SetKeyState(int, bool);
	void SetUserCallback(dUserKeyCallbackType);
	dUserKeyCallbackType GetUserCallback();
	size_t AddRuntime(Runtime*);
	void RemoveRuntime(size_t runtimeIndex);
private:
	Input();
private:
	bool m_started;
	LockData m_lockData;
	dUserKeyCallbackType m_userCallback;
	ReciclingVector<Runtime*> m_runtimes;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

typedef
struct KeyStateBuffers
{
	static constexpr size_t numberOfKeys{ Input::numberOfKeys };

	std::array<bool, numberOfKeys> KeysPressed;
	std::array<bool, numberOfKeys> KeysPressedThisFrame;
	std::array<bool, numberOfKeys> KeysReleasedThisFrame;
} KeyStateBuffers;

}
