#include "Runtime.h"
#include "AssetManager.h"
#include "DCoreAssert.h"
#include "ComponentRef.h"
#include "DCoreMath.h"
#include "PerspectiveCameraComponent.h"
#include "ReadWriteLockGuard.h"
#include "Timer.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "ECSTypes.h"
#include "SerializationTypes.h"
#include "Quad.h"
#include "AnimationStateMachineComponent.h"
#include "BoxColliderComponent.h"
#include "PhysicsAPI.h"
#include "ScriptComponent.h"
#include "ChildrenComponent.h"
#include "Sound.h"
#include "SceneLoader.h"

#include "box2d/types.h"



namespace DCore
{

struct RayCastContext
{
	DCore::Runtime* Runtime;
	Physics::RayCastResult Result;
};

struct OverlapContext
{
	DCore::Runtime* Runtime;
	Physics::OverlapResult* Result;
	size_t MaxEntitiesSize;
	size_t EntitiesSize;
	bool Overlap;
};

}

static float RayCastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
{
	DCore::RayCastContext* rayCastContext(static_cast<DCore::RayCastContext*>(context));
	DASSERT_E(rayCastContext != nullptr);
	const DCore::UserData& userData(rayCastContext->Runtime->GetUserDataAtIndex(reinterpret_cast<size_t>(b2Shape_GetUserData(shapeId))));
	rayCastContext->Result.Entity = userData.Entity;
	rayCastContext->Result.Point = {point.x, point.y};
	rayCastContext->Result.Normal = {normal.x, normal.y};
	rayCastContext->Result.Hit = true;
	return fraction;
}

static bool OverlapCallback(b2ShapeId shapeId, void* context)
{
	DCore::OverlapContext* overlapContext(static_cast<DCore::OverlapContext*>(context));
	overlapContext->Overlap = true;
	if (overlapContext->Result == nullptr)
	{
		return false;
	}
	const DCore::UserData& userData(overlapContext->Runtime->GetUserDataAtIndex(reinterpret_cast<size_t>(b2Shape_GetUserData(shapeId))));
	overlapContext->Result->Entities[overlapContext->EntitiesSize++] = userData.Entity;
	return overlapContext->EntitiesSize < overlapContext->MaxEntitiesSize;
}

namespace DCore
{

Runtime::Runtime(GLFWwindow* context)
	:
	m_toContinueSimulation(false),
	m_currentState(RuntimeState::NotPlaying),
	m_physicsWorldId(b2_nullWorldId),
	m_nextAsyncContext(0)
{
	Input::Get().Start(context);
	glfwWindowHint(GLFW_VISIBLE, false);
	m_context = glfwCreateWindow(800, 600, "Offscreen window", nullptr, context);
	for (size_t i(0); i < maximumNumberOfScenesLoadedAsync; i++)
	{
		AsyncSceneContext asyncSceneContext;
		asyncSceneContext.Context = glfwCreateWindow(800, 600, "Offscreen window", nullptr, context);
		m_asyncSceneContexts.PushBack(std::move(asyncSceneContext));
	}
	DASSERT_E(m_context != nullptr);
}

Runtime::~Runtime()
{
	m_toContinueSimulation.store(false, std::memory_order_relaxed);
	if (m_currentState == RuntimeState::Playing)
	{
		DASSERT_E(m_gameLoopThread.joinable());
		m_gameLoopThread.join();
		for (AsyncSceneContext& context : m_asyncSceneContexts)
		{
			if (context.Context != nullptr)
			{
				glfwDestroyWindow(context.Context);
			}
		}
	}
	if (m_context != nullptr)
	{
		glfwDestroyWindow(m_context);
	}
}

void Runtime::MakeDefaultRendererSubmitions(const DVec2& viewportSizes, Renderer& renderer, const drawDebugBoxCommandContainerType* drawDebugBoxCommads)
{
	DMat4 viewProjectionMatrix;
	bool cameraFound(false);
	ReadWriteLockGuard guard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	AssetManager::Get().IterateOnLoadedScenes
	(
		[&](SceneRef sceneRef) -> bool
		{
			if (!sceneRef.IsLoaded())
			{
				return false;
			}
			sceneRef.Iterate<TransformComponent, PerspectiveCameraComponent>
			(
				[&](Entity, ComponentRef<TransformComponent> transformComponent, ComponentRef<PerspectiveCameraComponent> perspectiveCameraComponent) -> bool
				{
					viewProjectionMatrix = perspectiveCameraComponent.GetProjectionMatrix(viewportSizes) * glm::inverse(transformComponent.GetModelMatrix());
					cameraFound = true;
					return true;
				}
			);
			if (cameraFound)
			{
				return true;
			}
			// TODO. Search for a orthographic camera
			return false;
		}
	);
	if (!cameraFound)
	{
		return;
	}
	MakeRendererSubmitions(viewProjectionMatrix, renderer, drawDebugBoxCommads);
}

void Runtime::MakeRendererSubmitions(const DMat4& viewProjectionMatrix, Renderer& renderer, const drawDebugBoxCommandContainerType* drawDebugBoxCommands)
{
	AssetManager::Get().IterateOnLoadedScenes
	(
		[&](SceneRef sceneRef) -> bool
		{
			if (!sceneRef.IsLoaded())
			{
				return false;
			}
			sceneRef.Iterate<TransformComponent, SpriteComponent>
			(
				[&](Entity entity, ComponentRef<TransformComponent> transformComponent, ComponentRef<SpriteComponent> spriteComponent) -> bool
				{
					if (!spriteComponent.IsEnabled())
					{
						return false;
					}
					const EntityRef entityRef(entity, sceneRef);
					DMat4 mvp(viewProjectionMatrix);
					mvp *= entityRef.HaveParent() ? entityRef.GetWorldModelMatrix() : transformComponent.GetModelMatrix();
					ReadWriteLockGuard materialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
					ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
					Quad3 vertexPostions;
					const Quad2 spriteVertexPositions(spriteComponent.GetCurrentSpriteVertexPositions());
					vertexPostions.BottomLeft = {spriteVertexPositions.BottomLeft.x, spriteVertexPositions.BottomLeft.y, 0.0f};
					vertexPostions.BottomRight = {spriteVertexPositions.BottomRight.x, spriteVertexPositions.BottomRight.y, 0.0f};
					vertexPostions.TopRight = {spriteVertexPositions.TopRight.x, spriteVertexPositions.TopRight.y, 0.0f};
					vertexPostions.TopLeft = {spriteVertexPositions.TopLeft.x, spriteVertexPositions.TopLeft.y, 0.0f};
					const DVec4 diffuseColor(spriteComponent.GetDiffuseColor());
					const SceneIdType sceneId(sceneRef.GetInternalSceneRefId());
					const SceneVersionType sceneVersion(sceneRef.GetInternalSceneRefVersion());
					bool toUseDiffuseTexture(true);
					uint32_t diffuseTextureId(0);
					if (!spriteComponent.GetSpriteMaterialRef().IsValid())
					{
						toUseDiffuseTexture = false;
					}
					else
					{
						if (spriteComponent.GetSpriteMaterialRef().GetDiffuseMapRef().IsValid())
						{
							diffuseTextureId = spriteComponent.GetSpriteMaterialRef().GetDiffuseMapRef().GetId();
						}
						else
						{
							toUseDiffuseTexture = false;
						}
					}
					const DCore::Quad2 spriteUvs(spriteComponent.GetCurrentSpriteUvs());
					const DCore::DVec4& tintColor(spriteComponent.GetTintColor());
					Renderer::unlitTexturedObjectRendererType::quadType quad;
					for (uint8_t index(0); index < 4; index++)
					{
						UnlitTexturedVertex& vertex(quad[index]);
						vertex.DrawOrder = spriteComponent.GetDrawOrder();
						vertex.MVP = mvp;
						vertex.VertexPos = vertexPostions.At(index);
						vertex.DiffuseColor = diffuseColor;
						vertex.TintColor = tintColor;
						vertex.ToUseDiffuseTex = toUseDiffuseTexture ? 1 : 0;
						vertex.DiffuseTexId = diffuseTextureId;
						vertex.UV = spriteUvs.At(index);
						vertex.EntityId = entity.GetId();
						vertex.EntityVersion = entity.GetVersion();
						vertex.SceneId = sceneId;
						vertex.SceneVersion = sceneVersion;
					}
					renderer.SubmitUnlitTexturedObject(quad);
					return false;
				}
			);
			sceneRef.Iterate<TransformComponent, BoxColliderComponent>
			(
				[&](Entity entity, ComponentRef<TransformComponent> transform, ComponentRef<BoxColliderComponent> boxCollider) -> bool
				{
					if (!boxCollider.HaveToDrawCollider())
					{
						return false;
					}
					const DVec3 translation(transform.GetTranslation());
					const DVec2 sizes(boxCollider.GetSizes());
					const DFloat rotation(transform.GetRotation());
					const DVec2 scale(transform.GetScale());
					const DVec2 offset(boxCollider.GetOffset());
					const TransformComponent transformComponent({{translation.x, translation.y, 0.0f}, rotation, scale});
					const EntityRef entityRef(entity, sceneRef);
					DMat4 mvp(viewProjectionMatrix);
					if (entityRef.HaveParent())
					{
						mvp *= entityRef.GetWorldModelMatrix();
					}
					else
					{
						mvp *= transformComponent.GetModelMatrix();
					}
					Renderer::debugRectObjectRenderer::objectType object;
					DebugRectVertex& vertex(object[0]);
					vertex.MVP = mvp;
					vertex.Offset = offset;
					vertex.RectSizes = sizes;
					vertex.Color = {0.0f, 1.0f, 0.0f, 1.0f};
					renderer.SubmitDebugRectObject(object);
					return false;
				}
			);
			return false;
		}
	);
	if (drawDebugBoxCommands != nullptr)
	{
		for (const DrawDebugBoxCommand& command : *drawDebugBoxCommands)
		{
			const DMat4 mvp(viewProjectionMatrix * command.Model);
			Renderer::debugRectObjectRenderer::objectType object;
			DebugRectVertex& vertex(object[0]);
			vertex.MVP = mvp;
			vertex.Offset = {0.0f, 0.0f};
			vertex.RectSizes = command.Sizes;
			vertex.Color = command.Color;
			renderer.SubmitDebugRectObject(object);
		}
	}
}

void Runtime::SetupEntityPhysics(EntityRef entity, Runtime& runtime)
{
	runtime.SetupEntityPhysics(entity);
}

void Runtime::Begin()
{
	if (m_currentState == RuntimeState::Playing)
	{
		return;
	}
	m_currentState = RuntimeState::Playing;
	m_gameLoopThread = std::thread(&Runtime::GameLoop, this);
	m_toContinueSimulation.store(true, std::memory_order_relaxed);
	m_inputIndex = Input::Get().AddRuntime(this);
}

void Runtime::End()
{
	if (m_currentState == RuntimeState::NotPlaying)
	{
		return;
	}
	m_currentState = RuntimeState::NotPlaying;
	m_toContinueSimulation.store(false, std::memory_order_relaxed);
	m_nextAsyncContext = 0;
	DASSERT_E(m_gameLoopThread.joinable());
	m_gameLoopThread.join();
	Input::Get().RemoveRuntime(m_inputIndex);
}

void Runtime::SetRendererViewportSizes(const DVec2& sizes)
{
	m_rendererViewportSizes.Set(sizes.x, sizes.y);
}

void Runtime::DestroyEntity(EntityRef entity)
{
	ReadWriteLockGuard runtimeGuard(LockType::WriteLock, m_lockData);
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	if (m_currentState == RuntimeState::NotPlaying)
	{
		return;
	}
	DestroyEntityNoLock(entity);
}

void Runtime::Render(const DVec2& viewportSizes, Renderer& renderer)
{
	DASSERT_E(viewportSizes.x > 0.0f && viewportSizes.y > 0.0f && renderer.IsRenderingDone());
	ReadWriteLockGuard readGuard(LockType::ReadLock, m_lockData);
	renderer.Begin(viewportSizes);
	MakeDefaultRendererSubmitions(viewportSizes, renderer, &m_drawDebugBoxCommands);
	ReadWriteLockGuard writeGuard(LockType::WriteLock, m_lockData);
	m_drawDebugBoxCommands.Clear();
	renderer.Render();
}

size_t Runtime::RegisterToOnCollisionBegin(ComponentRef<ScriptComponent> scriptComponent, DBodyId bodyId)
{
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	UserData::scriptComponentContainerType::ConstRef ref(userData.CollisionBeginScriptComponents.PushBack(scriptComponent));
	return ref.GetId() - 1;
}

size_t Runtime::RegisterToOnCollisionEnd(ComponentRef<ScriptComponent> scriptComponent, DBodyId bodyId)
{
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	UserData::scriptComponentContainerType::ConstRef ref(userData.CollisionEndScriptComponents.PushBack(scriptComponent));
	return ref.GetId() - 1;
}

size_t Runtime::RegisterToOnOverlapBegin(ComponentRef<ScriptComponent> scriptComponent, DBodyId bodyId)
{
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	UserData::scriptComponentContainerType::ConstRef ref(userData.OverlapBeginScriptComponents.PushBack(scriptComponent));
	return ref.GetId() - 1;
}

size_t Runtime::RegisterToOnOverlapEnd(ComponentRef<ScriptComponent> scriptComponent, DBodyId bodyId)
{
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	UserData::scriptComponentContainerType::ConstRef ref(userData.OverlapEndScriptComponents.PushBack(scriptComponent));
	return ref.GetId() - 1;
}

void Runtime::RemoveFromOnCollisionBegin(size_t registrationIndex, DBodyId bodyId)
{
	if (!b2Body_IsValid(bodyId))
	{
		return;
	}
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	userData.CollisionBeginScriptComponents.RemoveElementAtIndex(registrationIndex);
}


void Runtime::RemoveFromOnCollisionEnd(size_t registrationIndex, DBodyId bodyId)
{
	if (!b2Body_IsValid(bodyId))
	{
		return;
	}
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	userData.CollisionEndScriptComponents.RemoveElementAtIndex(registrationIndex);
}

void Runtime::RemoveFromOnOverlapBegin(size_t registrationIndex, DBodyId bodyId)
{
	if (!b2Body_IsValid(bodyId))
	{
		return;
	}
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	userData.OverlapBeginScriptComponents.RemoveElementAtIndex(registrationIndex);
}


void Runtime::RemoveFromOnOverlapEnd(size_t registrationIndex, DBodyId bodyId)
{
	if (!b2Body_IsValid(bodyId))
	{
		return;
	}
	void* index(b2Body_GetUserData(bodyId));
	UserData& userData(m_userDatas[reinterpret_cast<size_t>(index)]);
	userData.OverlapEndScriptComponents.RemoveElementAtIndex(registrationIndex);
}

bool Runtime::RayCastBox(float boxRotation, const DVec2& boxSizes, const DVec2& origin, const DVec2& direction, float maxDistance, uint64_t onlyColliderWithLayers, rayCastResultType* out)
{
	RayCastContext rayCastContext;
	rayCastContext.Runtime = this;
	rayCastContext.Result.Hit = false;
	b2Polygon boxPolygon(b2MakeOffsetBox(boxSizes.x/2.0f, boxSizes.y/2.0f, {0.0f, 0.0f}, b2MakeRot(glm::radians(boxRotation))));
	b2Transform transform;
	transform.p = {origin.x, origin.y};
	transform.q = b2Rot_identity;
	b2QueryFilter filter(b2DefaultQueryFilter());
	filter.maskBits = onlyColliderWithLayers;
	b2World_CastPolygon(m_physicsWorldId, &boxPolygon, transform, {direction.x * maxDistance, direction.y * maxDistance}, filter, &RayCastCallback, &rayCastContext);
	if (out != nullptr)
	{
		*out = rayCastContext.Result;
	}
	return rayCastContext.Result.Hit;
}

bool Runtime::OverlapBox(float boxRotation, const DVec2& boxSizes, const DVec2& origin, uint64_t selfPhysicsLayer, uint64_t onlyCollideWithLayers, overlapResultType* result, size_t entitiesSize)
{
	DASSERT_E(result == nullptr || (result != nullptr && entitiesSize > 0));
	OverlapContext context;
	context.Runtime = this;
	context.Result = result;
	context.MaxEntitiesSize = entitiesSize;
	context.EntitiesSize = 0;
	context.Overlap = false;
	b2Polygon boxPolygon(b2MakeOffsetBox(boxSizes.x/2.0f, boxSizes.y/2.0f, {0.0f, 0.0f}, b2MakeRot(glm::radians(boxRotation))));
	b2Transform transform;
	transform.p = {origin.x, origin.y};
	transform.q = b2Rot_identity;
	b2QueryFilter filter(b2DefaultQueryFilter());
	filter.categoryBits = selfPhysicsLayer;
	filter.maskBits = onlyCollideWithLayers;
	b2World_OverlapPolygon(m_physicsWorldId, &boxPolygon, transform, filter, &OverlapCallback, &context);
	return context.Overlap;
}

void Runtime::AddDrawDebugBoxCommand(const DrawDebugBoxCommand& command)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	m_drawDebugBoxCommands.PushBack(command);
}

void Runtime::SetSceneToUnload(const stringType& sceneName)
{
	m_namesOfScenesToUnload.push_back(sceneName);
}

void Runtime::SetSceneToLoad(const stringType& sceneName)
{
	m_namesOfScenesToLoad.push_back(sceneName);
}

void Runtime::SetSceneToLoadAsync(const stringType& sceneName)
{
	m_namesOfScenesToLoadAsync.push_back(sceneName);
}

void Runtime::AddKeyEvent(KeyEvent event)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	m_keyEvents.push_back(event);
}

bool Runtime::KeyPressed(DKey key)
{
	return m_keyStateBuffers.KeysPressed[KEY_TO_INT(key)];
}

bool Runtime::KeyPressedThisFrame(DKey key)
{
	return m_keyStateBuffers.KeysPressedThisFrame[KEY_TO_INT(key)];
}

bool Runtime::KeyReleasedThisFrame(DKey key)
{
	return m_keyStateBuffers.KeysReleasedThisFrame[KEY_TO_INT(key)];
}

void Runtime::GameLoop()
{
	constexpr float physicsDeltaTime{1.0f/60.0f};
	constexpr float animationDeltaTime{1.0f/30.0f};
	constexpr float biggestDeltaTime{animationDeltaTime};
	glfwMakeContextCurrent(m_context);
	float physicsAccumulatedDeltaTime{0.0f};
	float animationAccumulatedDeltaTime{0.0f};
	float lastTime(0.0f);
	float deltaTime(1.0f/60.0f);
	SetupKeyStateBuffers();
	AnimationSetup();
	SetupPhysics();
	AwakeScripts();
	StartScripts();
	while (m_toContinueSimulation.load(std::memory_order_relaxed))
	{
		//Timer<std::chrono::microseconds> timer(" Runtime loop");
		UpdateInput();
		Sound::Get().Update3DAudioListener();
		SetupScenesLoadedAsync();
		UpdateScripts(deltaTime);
		if (physicsAccumulatedDeltaTime >= physicsDeltaTime)
		{
			physicsAccumulatedDeltaTime = 0.0f;
			PhysicsUpdateScripts(physicsDeltaTime);
			PhysicsUpdate(physicsDeltaTime);
			PhysicsLateUpdateScripts(physicsDeltaTime);
		}
		if (animationAccumulatedDeltaTime >= animationDeltaTime)
		{
			animationAccumulatedDeltaTime = 0.0f;
			AnimationUpdateScripts(animationDeltaTime);
			AnimationUpdate(animationDeltaTime);
		}
		LateUpdateScripts(deltaTime);
		Sound::Get().Update();
		float currentTime(glfwGetTime());
		deltaTime = currentTime - lastTime;
		if (deltaTime > biggestDeltaTime)
		{
			deltaTime = biggestDeltaTime;
		}
		lastTime = currentTime;
		physicsAccumulatedDeltaTime += deltaTime;
		animationAccumulatedDeltaTime += deltaTime;
		for (const stringType& sceneName : m_namesOfScenesToUnload)
		{
			for (AsyncSceneContext& context : m_asyncSceneContexts)
			{
				if (context.LoadingThread.joinable())
				{
					context.LoadingThread.join();
				}
			}
			m_nextAsyncContext = 0;
			UnloadScene(sceneName);
		}
		if (!m_namesOfScenesToUnload.empty())
		{
			m_namesOfScenesToUnload.clear();
		}
		for (const stringType& sceneName : m_namesOfScenesToLoad)
		{
			SceneRef scene(SceneLoader::Get().LoadScene(sceneName));
			SetupScene(scene);
		}
		if (!m_namesOfScenesToLoad.empty())
		{
			m_namesOfScenesToLoad.clear();
		}
		for (const stringType& sceneName : m_namesOfScenesToLoadAsync)
		{
			if (m_nextAsyncContext >= maximumNumberOfScenesLoadedAsync)
			{
				m_nextAsyncContext = 0;
			}
			AsyncSceneContext& context(m_asyncSceneContexts[m_nextAsyncContext++]);
			if (context.LoadingThread.joinable())
			{
				context.LoadingThread.join();
			}
			context.LoadingDone = false;
			context.AtomicLoadingDone = false;
			context.LoadingThread = std::thread(&Runtime::LoadSceneAsync, this, sceneName, &context);
		}
		if (!m_namesOfScenesToLoadAsync.empty())
		{
			m_namesOfScenesToLoadAsync.clear();
		}
	}
	for (AsyncSceneContext& context : m_asyncSceneContexts)
	{
		if (context.LoadingThread.joinable())
		{
			context.LoadingThread.join();
		}
		context.LoadingDone = true;
	}
	TerminateEntities();
	b2DestroyWorld(m_physicsWorldId);
	m_physicsWorldId = b2_nullWorldId;
	Sound::Get().Update();
}

void Runtime::SetupPhysics()
{
	b2WorldDef worldDef(b2DefaultWorldDef());
	m_physicsWorldId = b2CreateWorld(&worldDef);
	m_userDatas.Clear();
	ReadWriteLockGuard runtimeGuard(LockType::WriteLock, m_lockData);
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	AssetManager::Get().IterateOnLoadedScenes
	(
		[&](SceneRef scene) -> bool
		{
			// TODO. Considerar outros componentes de colisores.
			scene.Iterate<TransformComponent, BoxColliderComponent>
			(
				[&](Entity entity, ComponentRef<TransformComponent> transform, ComponentRef<BoxColliderComponent> boxCollider) -> bool
				{
					SetupEntityPhysics({entity, scene});
					return false;
				}
			);
			return false;
		}
	);
}

void Runtime::SetupEntityPhysics(EntityRef entity)
{
	auto [transform, boxCollider] = entity.GetComponents<TransformComponent, BoxColliderComponent>();
	b2BodyDef bodyDef(b2DefaultBodyDef());
	const DMat4 modelMatrix(entity.GetWorldModelMatrix());
	DVec2 translation, scale;
	float rotation;
	Math::Decompose(modelMatrix, translation, rotation, scale);
	bodyDef.type = Physics::CoreBodyTypeToBox2dBodyType(boxCollider.GetBodyType());
	bodyDef.position = {translation.x, translation.y};	
	bodyDef.rotation = b2MakeRot(glm::radians(rotation));
	bodyDef.isEnabled = boxCollider.IsEnabled();
	bodyDef.enableSleep = false;
	bodyDef.fixedRotation = boxCollider.IsRotationFixed();
	bodyDef.gravityScale = boxCollider.GetGravityScale();
	userDataContainerType::Ref userData(m_userDatas.PushBack(entity));
	userData->Index = userData.GetIndex();
	bodyDef.userData = reinterpret_cast<void*>(userData.GetId() - 1);
	b2BodyId bodyId(b2CreateBody(m_physicsWorldId, &bodyDef));
	boxCollider.SetBodyId(bodyId);
	const DVec2 boxColliderSizes(boxCollider.GetSizes());
	const DVec2 boxColliderOffset(boxCollider.GetOffset());
	b2Polygon polygon(b2MakeOffsetBox(glm::abs(scale.x * boxColliderSizes.x/2.0f), glm::abs(scale.y * boxColliderSizes.y/2.0f), {scale.x * boxColliderOffset.x, scale.y * boxColliderOffset.y}, b2Rot_identity));
	b2ShapeDef shapeDef(b2DefaultShapeDef());
	shapeDef.isSensor = boxCollider.IsSensor();
	shapeDef.enableContactEvents = true;
	const Physics::PhysicsLayer selfPhysicsLayer(boxCollider.GetSelfPhysicsLayer());
	const Physics::PhysicsLayer collideWithPhysicsPlayers(boxCollider.GetCollideWithPhysicsLayers());
	shapeDef.filter.categoryBits = selfPhysicsLayer == Physics::PhysicsLayer::Unspecified ? B2_DEFAULT_CATEGORY_BITS : static_cast<uint64_t>(selfPhysicsLayer);
	shapeDef.filter.maskBits = collideWithPhysicsPlayers == Physics::PhysicsLayer::Unspecified ? B2_DEFAULT_MASK_BITS : static_cast<uint64_t>(collideWithPhysicsPlayers);
	shapeDef.userData = bodyDef.userData;
	const PhysicsMaterialRef physicsMaterial(boxCollider.GetPhysicsMaterial());
	ReadWriteLockGuard guard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	if (physicsMaterial.IsValid())
	{
		shapeDef.density = physicsMaterial.GetDensity();
		shapeDef.friction = physicsMaterial.GetFriction();
		shapeDef.restitution = physicsMaterial.GetRestitution();
	}
	boxCollider.SetShapeId(b2CreatePolygonShape(bodyId, &shapeDef, &polygon));
}

void Runtime::SetupKeyStateBuffers()
{
	for (size_t i(0); i < KeyStateBuffers::numberOfKeys; i++)
	{
		m_keyStateBuffers.KeysPressed[i] = false;
		m_keyStateBuffers.KeysPressedThisFrame[i] = false;
		m_keyStateBuffers.KeysReleasedThisFrame[i] = false;
	}
}

void Runtime::AwakeScripts()
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.SetRuntime(this);
						scriptComponent.Awake();
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::StartScripts()
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.Start();
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::UpdateScripts(float deltaTime)
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.Update(deltaTime);
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::LateUpdateScripts(float deltaTime)
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.LateUpdate(deltaTime);
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::PhysicsUpdateScripts(float physicsDeltaTime)
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.PhysicsUpdate(physicsDeltaTime);
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::PhysicsLateUpdateScripts(float physicsDeltaTime)
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.PhysicsLateUpdate(physicsDeltaTime);
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::AnimationUpdateScripts(float animationDeltaTime)
{
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				scene.Iterate
				(
					&scriptComponentId, 1,
					[&](Entity, ComponentRef<ScriptComponent> scriptComponent) -> bool
					{
						scriptComponent.AnimationUpdate(animationDeltaTime);
						return false;
					}
				);
			}
			return false;
		});
}

void Runtime::PhysicsUpdate(float physicsDeltaTime)
{
	constexpr int subStepCount{4};
	ReadWriteLockGuard runtimeGuard(LockType::ReadLock, m_lockData);
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			scene.Iterate<TransformComponent>
			(
				[&](Entity entity, ComponentRef<TransformComponent> transform) -> bool
				{
					EntityRef entityRef(entity, scene);
					UpdateEntityPhysics(entityRef, transform);
					return false;
				}
			);
			return false;	
		});
	b2World_Step(m_physicsWorldId, physicsDeltaTime, subStepCount);
	b2BodyEvents bodyEvents(b2World_GetBodyEvents(m_physicsWorldId));
	b2ContactEvents contactEvents(b2World_GetContactEvents(m_physicsWorldId));
	b2SensorEvents sensorEvents(b2World_GetSensorEvents(m_physicsWorldId));
	// Transform update
	for (size_t i(0); i < bodyEvents.moveCount; i++)
	{
		const b2BodyMoveEvent* event(bodyEvents.moveEvents + i);
		UserData& userData(m_userDatas[reinterpret_cast<size_t>(event->userData)]);
		if (!userData.Entity.IsValid())
		{
			continue;
		}
		ComponentRef<TransformComponent> transform(userData.Entity.GetComponents<TransformComponent>());
		DASSERT_E(transform.IsValid());
		float b2Rotation(glm::degrees(b2Rot_GetAngle(event->transform.q)));
		if (b2Rotation > 90.0f)
		{
			b2Rotation -= 180.0f;
		}
		else if (b2Rotation < -90.0f)
		{
			b2Rotation += 180.0f;
		}
		TransformComponent toWorldTransform({{event->transform.p.x, event->transform.p.y, 0.0f}, b2Rotation, transform.GetScale()});
		DMat4 toWorldModelMatrix(toWorldTransform.GetModelMatrix());
		userData.Entity.RemoveParentTransformationsFrom(toWorldModelMatrix);
		DVec2 translation, scale;
		DFloat rotation;
		Math::Decompose(toWorldModelMatrix, translation, rotation, scale);
		transform.SetTranslation({translation.x, translation.y, transform.GetTranslation().z});
		transform.SetRotation(rotation);
		//transform.SetScale(scale);
		transform.SetIsDirty(false);
	}
	// Collision begin callbacks.
	for (size_t i(0); i < contactEvents.beginCount; i++)
	{
		b2ContactBeginTouchEvent* beginEvent(contactEvents.beginEvents + i);
		if (!b2Shape_IsValid(beginEvent->shapeIdA) || !b2Shape_IsValid(beginEvent->shapeIdB))
		{
			continue;
		}
		UserData& userDataA(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(beginEvent->shapeIdA))]);
		UserData& userDataB(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(beginEvent->shapeIdB))]);
		userDataA.CollisionBeginScriptComponents.Iterate
		(
			[&](UserData::scriptComponentContainerType::Ref scriptComponent) -> bool
			{
				scriptComponent->OnCollisionBegin(userDataB.Entity);
				return false;
			}
		);
		userDataB.CollisionBeginScriptComponents.Iterate
		(
			[&](UserData::scriptComponentContainerType::Ref scriptComponent) -> bool
			{
				scriptComponent->OnCollisionBegin(userDataA.Entity);
				return false;
			}
		);
	}
	// Collision end callbaks.
	for (size_t i(0); i < contactEvents.endCount; i++)
	{
		b2ContactEndTouchEvent* endEvent(contactEvents.endEvents + i);
		if (!b2Shape_IsValid(endEvent->shapeIdA) || !b2Shape_IsValid(endEvent->shapeIdB))
		{
			continue;
		}
		UserData& userDataA(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(endEvent->shapeIdA))]);
		UserData& userDataB(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(endEvent->shapeIdB))]);
		userDataA.CollisionEndScriptComponents.Iterate
		(
			[&](UserData::scriptComponentContainerType::Ref scriptComponent) -> bool
			{
				scriptComponent->OnCollisionEnd(userDataB.Entity);
				return false;
			}
		);
		userDataB.CollisionEndScriptComponents.Iterate
		(
			[&](UserData::scriptComponentContainerType::Ref scriptComponent) -> bool
			{
				scriptComponent->OnCollisionEnd(userDataA.Entity);
				return false;
			}
		);
	}
	// Overlap begin callbaks.
	for (size_t i(0); i < sensorEvents.beginCount; i++)
	{
		b2SensorBeginTouchEvent* beginEvent(sensorEvents.beginEvents + i);
		if (!b2Shape_IsValid(beginEvent->sensorShapeId) || !b2Shape_IsValid(beginEvent->visitorShapeId))
		{
			continue;
		}
		UserData& userData(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(beginEvent->sensorShapeId))]);
		UserData& visitorUserData(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(beginEvent->visitorShapeId))]);
		userData.OverlapBeginScriptComponents.Iterate
		(
			[&](UserData::scriptComponentContainerType::Ref scriptComponent) -> bool
			{
				scriptComponent->OnOverlapBegin(visitorUserData.Entity);
				return false;
			}
		);
	}
	// Overlap end callbacks.
	for (size_t i(0); i < sensorEvents.endCount; i++)
	{
		b2SensorEndTouchEvent* endEvent(sensorEvents.endEvents + i);
		if (!b2Shape_IsValid(endEvent->sensorShapeId) || !b2Shape_IsValid(endEvent->visitorShapeId))
		{
			continue;
		}
		UserData& userData(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(endEvent->sensorShapeId))]);
		UserData& visitorUserData(m_userDatas[reinterpret_cast<size_t>(b2Shape_GetUserData(endEvent->visitorShapeId))]);
		userData.OverlapEndScriptComponents.Iterate
		(
			[&](UserData::scriptComponentContainerType::Ref scriptComponent) -> bool
			{
				scriptComponent->OnOverlapEnd(visitorUserData.Entity);
				return false;
			}
		);
	}
}

void Runtime::UpdateEntityPhysics(EntityRef entityRef, ComponentRef<TransformComponent> transform)
{
	ComponentRef<BoxColliderComponent> boxCollider;
	if (entityRef.HaveComponents<BoxColliderComponent>())
	{
		boxCollider = entityRef.GetComponents<BoxColliderComponent>();
	}
	if (transform.IsDirty())
	{
		const DMat4 modelMatrix(entityRef.GetWorldModelMatrix());
		DVec2 translation, scale;
		DFloat rotation;
		Math::Decompose(modelMatrix, translation, rotation, scale);
		if (boxCollider.IsValid())
		{
			b2BodyId bodyId(boxCollider.GetBodyId());
			b2Body_SetTransform(bodyId, {translation.x, translation.y}, b2MakeRot(glm::radians(rotation + (scale.x < 0.0f && scale.y < 0.0f ? 180.0f : 0.0f))));
		}
		if (entityRef.HaveChildren())
		{
			entityRef.IterateOnChildren
			(
				[&](EntityRef child) -> bool
				{
					ComponentRef<TransformComponent> childTransform(child.GetComponents<TransformComponent>());
					childTransform.SetIsDirty(true);
					UpdateEntityPhysics(child, childTransform);
					return false;
				}
			);
		}
		transform.SetIsDirty(false);
	}
	if (boxCollider.IsValid() && boxCollider.GetDirtyType() != 0)
	{
		const BoxColliderDirtyT dirtyType(boxCollider.GetDirtyType());
		const b2BodyId bodyId(boxCollider.GetBodyId());
		const b2ShapeId shapeId(boxCollider.GetShapeId());
		if (dirtyType & BoxColliderComponentDirtyType::BodyType)
		{
			b2Body_SetType(bodyId, Physics::CoreBodyTypeToBox2dBodyType(boxCollider.GetBodyType()));
		}
		if (dirtyType & BoxColliderComponentDirtyType::Enabled)
		{
			if (boxCollider.IsEnabled())
			{
				b2Body_Enable(bodyId);
			}
			else
			{
				b2Body_Disable(bodyId);
			}
		}
		if (dirtyType & BoxColliderComponentDirtyType::Sensor)
		{
			//b2Shape_EnableSensorEvents(shapeId, boxCollider.IsSensor());
		}
		if (dirtyType & BoxColliderComponentDirtyType::GravityScale)
		{
			b2Body_SetGravityScale(bodyId, boxCollider.GetGravityScale());
		}
		if (dirtyType & BoxColliderComponentDirtyType::FixedRotation)
		{
			b2Body_SetFixedRotation(bodyId, boxCollider.IsRotationFixed());
		}
		if (dirtyType & BoxColliderComponentDirtyType::UseCCD)
		{
			b2Body_SetBullet(bodyId, boxCollider.IsUsingCCD());
		}
		if ((dirtyType & BoxColliderComponentDirtyType::Offset) || 
				(dirtyType & BoxColliderComponentDirtyType::Size))
		{
			const DVec2 boxColliderSizes(boxCollider.GetSizes());
			const DVec2 boxColliderOffset(boxCollider.GetOffset());
			const DVec2 scale(transform.GetScale());
			b2Polygon polygon(b2MakeOffsetBox(scale.x * boxColliderSizes.x/2.0f, scale.y * boxColliderSizes.y/2.0f, {boxColliderOffset.x, boxColliderOffset.y}, b2Rot_identity));
			b2Shape_SetPolygon(shapeId, &polygon);
		}
		if (dirtyType & BoxColliderComponentDirtyType::PhysicsMaterial)
		{
			PhysicsMaterialRef physicsMaterial(boxCollider.GetPhysicsMaterial());
			if (physicsMaterial.IsValid())
			{
				b2Shape_SetDensity(shapeId, physicsMaterial.GetDensity(), true);
				b2Shape_SetFriction(shapeId, physicsMaterial.GetFriction());
				b2Shape_SetRestitution(shapeId, physicsMaterial.GetRestitution());
			}
		}
		if ((dirtyType & BoxColliderComponentDirtyType::SelfPhysicsLayer) ||
			(dirtyType & BoxColliderComponentDirtyType::ColliderWithPhysicsLayer))
		{
			b2Filter filter;
			const DPhysicsLayer selfPhysicsLayer(boxCollider.GetSelfPhysicsLayer());
			const DPhysicsLayer colliderWithPhysicsLayers(boxCollider.GetCollideWithPhysicsLayers());
			filter.categoryBits = selfPhysicsLayer == Physics::PhysicsLayer::Unspecified ? B2_DEFAULT_CATEGORY_BITS : static_cast<uint64_t>(selfPhysicsLayer);
			filter.maskBits = colliderWithPhysicsLayers == Physics::PhysicsLayer::Unspecified ? B2_DEFAULT_MASK_BITS : static_cast<uint64_t>(colliderWithPhysicsLayers);
			b2Shape_SetFilter(shapeId, filter);
			b2Body_SetType(bodyId, Physics::CoreBodyTypeToBox2dBodyType(boxCollider.GetBodyType()));
		}
		if (dirtyType & BoxColliderComponentDirtyType::LinearVelocity)
		{
			const DVec2 linearVelocity(boxCollider.GetLinearVelocity());
			b2Body_SetLinearVelocity(boxCollider.GetBodyId(), {linearVelocity.x, linearVelocity.y});						
		}
		boxCollider.Clean();
	}
}

void Runtime::AnimationSetup()
{
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			scene.Iterate<AnimationStateMachineComponent>(
				[&](Entity entity, ComponentRef<AnimationStateMachineComponent> asmComponent) -> bool
				{
					asmComponent.Setup();
					return false;
				});
			return false;
		});
}

void Runtime::AnimationUpdate(float deltaTime)
{
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			if (!scene.IsLoaded())
			{
				return false;
			}
			scene.Iterate<AnimationStateMachineComponent>(
				[&](Entity entity, ComponentRef<AnimationStateMachineComponent> asmComponent) -> bool
				{
					asmComponent.Tick(deltaTime);
					return false;
				});
			return false;
		});
}

void Runtime::UpdateInput()
{
	for (size_t i(0); i < KeyStateBuffers::numberOfKeys; i++)
	{
		m_keyStateBuffers.KeysPressedThisFrame[i] = false;
		m_keyStateBuffers.KeysReleasedThisFrame[i] = false;
	}
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	for (const KeyEvent& event : m_keyEvents)
	{
		switch (event.Event)
		{
		case KeyEventType::Pressed:
			m_keyStateBuffers.KeysPressed[event.Key] = true;
			m_keyStateBuffers.KeysPressedThisFrame[event.Key] = true;
			break;
		case KeyEventType::Released:
			m_keyStateBuffers.KeysPressed[event.Key] = false;
			m_keyStateBuffers.KeysReleasedThisFrame[event.Key] = true;
			break;
		default:
			break;
		}
	}
	m_keyEvents.clear();
}

void Runtime::DestroyEntityNoLock(EntityRef entity)
{
	if (entity.HaveComponents<BoxColliderComponent>())
	{
		ComponentRef<BoxColliderComponent> boxCollider(entity.GetComponents<BoxColliderComponent>());
		DBodyId bodyId(boxCollider.GetBodyId());
		UserData& userData(m_userDatas[reinterpret_cast<size_t>(b2Body_GetUserData(bodyId))]);
		m_userDatas.RemoveElementAtIndex(userData.Index);
		b2DestroyBody(bodyId);
	}
	if (!entity.HaveChildren())
	{
		return;
	}
	entity.IterateOnChildren
	(
		[&](EntityRef child) -> bool
		{
			DestroyEntityNoLock(child);
			return false;
		}
	);
}

void Runtime::TerminateEntities()
{
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes
	(
		[&](SceneRef scene) -> bool
		{
			scene.IterateOnEntities
			(
				[&](Entity entity) -> bool
				{
					EntityRef(entity, scene).Destroy();
					return false;
				}
			);
			return false;
		}
	);
}

void Runtime::UnloadScene(const stringType& sceneName)
{
	ReadWriteLockGuard runtimeGuard(LockType::WriteLock, m_lockData);
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	AssetManager::Get().IterateOnLoadedScenes(
		[&](SceneRef scene) -> bool
		{
			DString loadedSceneName;
			scene.GetName(loadedSceneName);
			if (loadedSceneName != sceneName.c_str())
			{
				return false;
			}
			scene.IterateOnEntities(
				[&](Entity entity) -> bool
				{
					EntityRef entityRef(entity, scene);
					DestroyEntityNoLock(entityRef);
					entityRef.Destroy();
					return false;
				});
			UUIDType uuid;
			scene.GetUUID(uuid);
			AssetManager::Get().UnloadScene(uuid);
			return true;
		});
}

void Runtime::LoadSceneAsync(stringType sceneName, AsyncSceneContext* context)
{
	glfwMakeContextCurrent(context->Context);
	context->LoadedScene = SceneLoader::Get().LoadScene(sceneName);
	context->AtomicLoadingDone.store(true, std::memory_order_release);
	glfwMakeContextCurrent(nullptr);
}

void Runtime::SetupScene(SceneRef scene)
{
	ReadWriteLockGuard runtimeGuard(LockType::WriteLock, m_lockData);
	ReadWriteLockGuard sceneGuard(LockType::ReadLock, *static_cast<SceneAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard asmGuard(LockType::ReadLock, *static_cast<AnimationStateMachineAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard animationGuard(LockType::ReadLock, *static_cast<AnimationAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard physicsMaterialGuard(LockType::ReadLock, *static_cast<PhysicsMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard spriteMaterialGuard(LockType::ReadLock, *static_cast<SpriteMaterialAssetManager*>(&AssetManager::Get()));
	ReadWriteLockGuard textureGuard(LockType::ReadLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
	scene.Iterate<AnimationStateMachineComponent>(
		[&](Entity entity, ComponentRef<AnimationStateMachineComponent> asmComponent) -> bool
		{
			asmComponent.Setup();
			return false;
		});
	scene.IterateOnEntities(
		[&](Entity entity) -> bool
		{
			EntityRef entityRef(entity, scene);
			if (entityRef.HaveComponents<BoxColliderComponent>())
			{
				SetupEntityPhysics(entityRef);
			}
			return false;
		});
	scene.IterateOnEntities(
		[&](Entity entity) -> bool
		{
			EntityRef entityRef(entity, scene);
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				ComponentRef<ScriptComponent> scriptComponent(entity, scene.GetInternalSceneRef(), scriptComponentId, *entityRef.GetLockData());
				if (!scriptComponent.IsValid())
				{
					continue;
				}
				scriptComponent.SetRuntime(this);
				scriptComponent.Awake();
			}
			return false;
		});
	scene.IterateOnEntities(
		[&](Entity entity) -> bool
		{
			EntityRef entityRef(entity, scene);
			for (ComponentIdType scriptComponentId : scriptComponentIds)
			{
				ComponentRef<ScriptComponent> scriptComponent(entity, scene.GetInternalSceneRef(), scriptComponentId, *entityRef.GetLockData());
				if (!scriptComponent.IsValid())
				{
					continue;
				}
				scriptComponent.Start();
			}
			return false;
		});
	scene.LoadingCompleted();
}

void Runtime::SetupScenesLoadedAsync()
{
	for (AsyncSceneContext& context : m_asyncSceneContexts)
	{
		// Frequent atomic loads uses too much processor. Do it only if non-atomic LoadingDone is false.
		if (context.LoadingDone)
		{
			continue;
		}
		if (!context.AtomicLoadingDone.load(std::memory_order_acquire))
		{
			continue;
		}
		context.LoadingDone = true;
		SetupScene(context.LoadedScene);
	}
}

}
