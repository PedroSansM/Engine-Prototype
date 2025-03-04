#pragma once

#include "Renderer.h"
#include "AtomicVec2.h"
#include "ReadWriteLockGuard.h"
#include "UserData.h"
#include "DebugDrawCommand.h"
#include "Array.h"
#include "PhysicsAPI.h"
#include "Input.h"

#include "box2d/types.h"
#include "box2d/box2d.h"

#include <atomic>
#include <thread>
#include <vector>
#include <float.h>
#include <string>
#include <unordered_map>



namespace DCore
{

class Runtime
{
public:
	static constexpr size_t drawDebugBoxCommandsSize{1024};
public:
	using rayCastResultType = Physics::RayCastResult;
	using overlapResultType = Physics::OverlapResult;
	using atomicBoolType = std::atomic_bool;
	using threadType = std::thread;
	using userDataContainerType = ReciclingVector<UserData>;
	using drawDebugBoxCommandContainerType = Array<DrawDebugBoxCommand, drawDebugBoxCommandsSize>;
	using stringType = std::string;
	using sceneNameContainerType = std::vector<stringType>;
	using keyEventContainerType = std::vector<KeyEvent>;
public:
	static constexpr size_t maximumNumberOfScenesLoadedAsync{64};
public:
	template <class Key, class Value>
	using unorderedMapType = std::unordered_map<Key, Value>;
public:
	Runtime(GLFWwindow* mainContext);
	Runtime(const Runtime&) = delete;
	Runtime(Runtime&&) = delete;
	~Runtime();
private:
	enum class RuntimeState
	{
		NotPlaying,
		Playing
	};
public:
	static void MakeDefaultRendererSubmitions(const DVec2& viewportSizes, Renderer&, const drawDebugBoxCommandContainerType* debugDrawBoxCommads = nullptr);

	// Requires that the SceneAssetManager be locked for reading.
	static void MakeRendererSubmitions(const DMat4& viewProjectionMatrix, Renderer&, const drawDebugBoxCommandContainerType* debugDrawBoxCommads = nullptr);

	// To be used by script components.
	static void SetupEntityPhysics(EntityRef, Runtime&);
public:
	void Begin();
	void End();
	void SetRendererViewportSizes(const DVec2&);
	void DestroyEntity(EntityRef);
	void Render(const DVec2& viewportSizes, Renderer&);
	// Physics.
	size_t RegisterToOnCollisionBegin(ComponentRef<ScriptComponent>, DBodyId);
	size_t RegisterToOnCollisionEnd(ComponentRef<ScriptComponent>, DBodyId);
	size_t RegisterToOnOverlapBegin(ComponentRef<ScriptComponent>, DBodyId);
	size_t RegisterToOnOverlapEnd(ComponentRef<ScriptComponent>, DBodyId);
	void RemoveFromOnCollisionBegin(size_t registrationIndex, DBodyId);
	void RemoveFromOnCollisionEnd(size_t componentIndex, DBodyId);
	void RemoveFromOnOverlapBegin(size_t registrationIndex, DBodyId);
	void RemoveFromOnOverlapEnd(size_t componentIndex, DBodyId);
	bool RayCastBox(
		float boxRotation, 
		const DVec2& boxSizes,
		const DVec2& origin, 
		const DVec2& direction, 
		float maxDistance = FLT_MAX, 
		uint64_t onlyCollideWithLayers = COLLIDE_WITH_ALL_MASK, 
		rayCastResultType* out = nullptr);
	bool OverlapBox(
		float boxRotation, 
		const DVec2& boxSizes, 
		const DVec2& origin, 
		uint64_t selfPhysicsLayer = UNDEFINED_PHYSICS_LAYER_MASK, 
		uint64_t onlyCollideWithLayers = COLLIDE_WITH_ALL_MASK, 
		overlapResultType* result = nullptr, 
		size_t entitiesSize = 0);
	// To be called only in scripts!
	void AddDrawDebugBoxCommand(const DrawDebugBoxCommand&);
	void SetSceneToUnload(const stringType& sceneName);
	void SetSceneToLoad(const stringType& sceneName);
	void SetSceneToLoadAsync(const stringType& sceneName);
	void AddKeyEvent(KeyEvent);
	bool KeyPressed(DKey);
	bool KeyPressedThisFrame(DKey);
	bool KeyReleasedThisFrame(DKey);
public:
	const UserData& GetUserDataAtIndex(size_t index) const
	{
		return m_userDatas[index];
	}
private:
	typedef
	struct AsyncSceneContext
	{
		AsyncSceneContext()
			:
			LoadingDone(true)
		{}

		AsyncSceneContext(struct AsyncSceneContext&& other) noexcept
			:
			Context(other.Context),
			LoadingThread(std::move(other.LoadingThread))
		{}

		GLFWwindow* Context;
		std::thread LoadingThread;
		bool LoadingDone;
		atomicBoolType AtomicLoadingDone;
		SceneRef LoadedScene;
	} AsyncSceneContext;
private:
	atomicBoolType m_toContinueSimulation;
	threadType m_gameLoopThread;
	AtomicVec2 m_rendererViewportSizes;
	RuntimeState m_currentState;
	b2WorldId m_physicsWorldId;
	LockData m_lockData;
	userDataContainerType m_userDatas;
	sceneNameContainerType m_namesOfScenesToUnload;
	sceneNameContainerType m_namesOfScenesToLoad;
	sceneNameContainerType m_namesOfScenesToLoadAsync;
	drawDebugBoxCommandContainerType m_drawDebugBoxCommands;
	GLFWwindow* m_context;
	Array<AsyncSceneContext, maximumNumberOfScenesLoadedAsync> m_asyncSceneContexts;
	size_t m_nextAsyncContext;
	size_t m_inputIndex;
	KeyStateBuffers m_keyStateBuffers;
	keyEventContainerType m_keyEvents;
private:
	void GameLoop();
	void SetupPhysics();
	void SetupEntityPhysics(EntityRef);
	void SetupKeyStateBuffers();
	void AwakeScripts();
	void StartScripts();
	void UpdateScripts(float deltaTime);
	void LateUpdateScripts(float deltaTime);
	void PhysicsUpdateScripts(float physicsDeltaTime);
	void PhysicsLateUpdateScripts(float physicsDeltaTime);
	void AnimationUpdateScripts(float animationDeltaTime);
	void PhysicsUpdate(float physicsDeltaTime);
	void UpdateEntityPhysics(EntityRef, ComponentRef<TransformComponent>);
	void AnimationSetup();
	void AnimationUpdate(float deltaTime);
	void UpdateInput();
	void DestroyEntityNoLock(EntityRef);
	void TerminateEntities();
	void UnloadScene(const stringType& sceneName);
	void LoadSceneAsync(stringType sceneName, AsyncSceneContext*);
	void SetupScene(SceneRef);
	void SetupScenesLoadedAsync();
};

}
