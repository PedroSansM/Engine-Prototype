#pragma once

#include "TemplateUtils.h"
#include "SerializationTypes.h"
#include "RendererTypes.h"
#include "SparseSet.h"
#include "UnlitSpriteMaterialShader.h"
#include "DebugShader.h"
#include "VertexStructures.h"
#include "UnlitTexturedObjectRenderer.h"
#include "DebugRectObjectRenderer.h"
#include "Graphics.h"

#include <array>
#include <atomic>
#include <thread>
#include <cstdint>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>



namespace DCore
{

class Renderer
{
public:
	static constexpr size_t maxNumberOfUnlitTexturedObjects{1000};
	static constexpr size_t maxNumberOfDebguRectObjects{1000};
public:
	using unlitTexturedObjectRendererType = UnlitTexturedObjectRenderer<maxNumberOfUnlitTexturedObjects>;
	using debugRectObjectRenderer = DebugRectObjectRenderer<maxNumberOfDebguRectObjects>;
	using mutexType = std::mutex;
	using conditionVariableType = std::condition_variable;
public:
	Renderer();
	~Renderer() = default;
private:
	enum class RenderStateEnum : uint8_t
	{
		UnlitSpriteMaterial,
		UnlitTexturedObject,
		LitSpriteMaterial
	};
	
	using RenderStateIndicator = struct RenderStateIndicator
	{
		size_t DrawOrder;
		SparseSet<uint8_t> RenderStates;

		RenderStateIndicator() = default;

		RenderStateIndicator(RenderStateIndicator&& other) noexcept
			:
			DrawOrder(other.DrawOrder),
			RenderStates(std::move(other.RenderStates))
		{}

		RenderStateIndicator& operator=(RenderStateIndicator&& other) noexcept
		{
			DrawOrder = other.DrawOrder;
			RenderStates = std::move(other.RenderStates);
			return *this;
		}
	};

	using RenderStateIndicatorComparator = struct RenderStateIndicatorComparator
	{
		bool operator()(const RenderStateIndicator& a, const RenderStateIndicator& b) const
		{
			return a.DrawOrder < b.DrawOrder;
		}
	};
public:
	void Initiate(GLFWwindow* mainWindow);
	void Terminate();
	void Begin(const DVec2& viewportSizes);
	void SubmitUnlitTexturedObject(const unlitTexturedObjectRendererType::quadType& vertices);
	void SubmitDebugRectObject(const debugRectObjectRenderer::objectType& vertices);
	void Render();
	bool TryReadPixelFromClickingTexture(const DVec2& clickPos, std::array<int, 4>& output);
public:
	bool IsRenderingDone() const
	{
		//return false;
		return m_isRenderingDone.load(std::memory_order_acquire);
	}

	unsigned int GetOutputTextureId() const
	{
		return m_outputTexture.load(std::memory_order_acquire);
	}

	// Should be called after Begin and before Render
	void SetClearColor(const DVec4& color)
	{
		m_clearColor[0] = color.r;
		m_clearColor[1] = color.g;
		m_clearColor[2] = color.b;
		m_clearColor[3] = color.a;
	}
private:
	std::atomic_bool m_isRenderingDone;
	bool m_submitionsDone;
	bool m_toTerminate;
	std::thread m_thread;
	GLFWwindow* m_context;
	DVec2 m_viewportSizes;
	std::atomic_uint m_outputTexture;
	std::vector<RenderStateIndicator> m_stateExecutionSequence;
	SparseSet<uint32_t> m_addedDrawOrders;
	bool m_clickRequest;
	std::array<int, 4> m_clickRequestData;
	DVec2 m_clickRequestPos;
	float m_clearColor[4];
	mutexType m_mutex;
	conditionVariableType m_conditionVariable;
private:
	unlitTexturedObjectRendererType m_unlitTexturesObjectRenderer;
	debugRectObjectRenderer m_debugRectObjectRenderer;
private:
	void RenderThread();
	void FlushBuffers();
};

}
