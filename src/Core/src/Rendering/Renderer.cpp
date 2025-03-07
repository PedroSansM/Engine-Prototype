#include "Renderer.h"
#include "DCoreAssert.h"
#include "RendererTypes.h"
#include "Timer.h"

#include <array>
#include <atomic>



namespace DCore
{

Renderer::Renderer()
	:
	m_isRenderingDone(true),
	m_submitionsDone(false),
	m_toTerminate(false),
	m_context(nullptr),
	m_outputTexture(0),
	m_clickRequest(false),
	m_clearColor{1.0f, 1.0f, 1.0f, 1.0f}
{}

void Renderer::Initiate(GLFWwindow* mainWindow)
{
	m_toTerminate.store(false, std::memory_order_relaxed);
	m_isRenderingDone.store(true, std::memory_order_relaxed);
	m_submitionsDone.store(false, std::memory_order_relaxed);
	m_clickRequest.store(false, std::memory_order_relaxed);
	glfwWindowHint(GLFW_VISIBLE, false);
	m_context = glfwCreateWindow(800, 600, "Offscreen Window", nullptr, mainWindow);
	DASSERT_E(m_context != nullptr);
	FlushBuffers();
	m_thread = std::thread(&Renderer::RenderThread, this);
}

void Renderer::Terminate()
{
	if (!m_thread.joinable())
	{
		return;
	}
	m_toTerminate.store(true, std::memory_order_relaxed);
	m_thread.join();
	if (m_context != nullptr)
	{
		glfwDestroyWindow(m_context);
	}
}

void Renderer::Begin(const DVec2& viewportSizes)
{
	DASSERT_E(m_isRenderingDone.load(std::memory_order_acquire));
	m_viewportSizes = viewportSizes;
	FlushBuffers();
	m_isRenderingDone.store(false, std::memory_order_release);
}

void Renderer::SubmitUnlitTexturedObject(const unlitTexturedObjectRendererType::quadType& vertices)
{
	const uint32_t drawOrder(vertices[0].DrawOrder);
	m_unlitTexturesObjectRenderer.Submit(vertices);
	if (!m_addedDrawOrders.Exists(drawOrder))
	{
		m_addedDrawOrders.Add(drawOrder);
		RenderStateIndicator renderStateIndicator;
		renderStateIndicator.DrawOrder = drawOrder;
		renderStateIndicator.RenderStates.Add(static_cast<uint8_t>(RenderStateEnum::UnlitTexturedObject));
		m_stateExecutionSequence.push_back(std::move(renderStateIndicator));
		return;
	}
	RenderStateIndicator& renderStateIndicator(m_stateExecutionSequence[m_addedDrawOrders.GetIndexTo(drawOrder)]);
	if (!renderStateIndicator.RenderStates.Exists(static_cast<uint8_t>(RenderStateEnum::UnlitTexturedObject)))
	{
		renderStateIndicator.RenderStates.Add(static_cast<uint8_t>(RenderStateEnum::UnlitTexturedObject));
	}
}

void Renderer::SubmitDebugRectObject(const debugRectObjectRenderer::objectType& vertices)
{
	m_debugRectObjectRenderer.Submit(vertices);
}

void Renderer::Render()
{
	m_submitionsDone.store(true, std::memory_order_release);
}

void Renderer::RenderThread()
{
	bool toSetup(true);
	GLuint outputFramebuffer1(0);
	GLuint outputFramebuffer2(0);
	GLuint outputTexture1(0);
	GLuint outputTexture2(0);
	GLuint clickingTexture1(0);
	GLuint clickingTexture2(0);
	m_outputTexture = outputTexture2;
	bool toDrawToFramebuffer1(true);
	static int clickingTextureClearColor[4]{-1, -1, -1, 1};
	while (!m_toTerminate.load(std::memory_order_relaxed))
	{
		if (toSetup)
		{
			glfwMakeContextCurrent(m_context);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			m_unlitTexturesObjectRenderer.Setup();
			m_debugRectObjectRenderer.Setup();
			GLenum drawBuffers[2]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			// Framebuffer 1
			glGenFramebuffers(1, &outputFramebuffer1); CHECK_GL_ERROR;
			glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer1); CHECK_GL_ERROR;
			glGenTextures(1, &outputTexture1); CHECK_GL_ERROR;
			glBindTexture(GL_TEXTURE_2D, outputTexture1); CHECK_GL_ERROR;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture1, 0); CHECK_GL_ERROR;
			glGenTextures(1, &clickingTexture1); CHECK_GL_ERROR;
			glBindTexture(GL_TEXTURE_2D, clickingTexture1); CHECK_GL_ERROR;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 800, 600, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, nullptr); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, clickingTexture1, 0); CHECK_GL_ERROR;
			glDrawBuffers(2, drawBuffers); CHECK_GL_ERROR;
			DASSERT_E(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE); CHECK_GL_ERROR;
			// 
			// Framebuffer 2
			glGenFramebuffers(1, &outputFramebuffer2); CHECK_GL_ERROR;
			glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer2); CHECK_GL_ERROR;
			glGenTextures(1, &outputTexture2); CHECK_GL_ERROR;
			glBindTexture(GL_TEXTURE_2D, outputTexture2); CHECK_GL_ERROR;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture2, 0); CHECK_GL_ERROR;
			glGenTextures(1, &clickingTexture2); CHECK_GL_ERROR;
			glBindTexture(GL_TEXTURE_2D, clickingTexture2); CHECK_GL_ERROR;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 800, 600, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, nullptr); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL_ERROR;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, clickingTexture2, 0); CHECK_GL_ERROR;
			glDrawBuffers(2, drawBuffers); CHECK_GL_ERROR;
			DASSERT_E(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE); CHECK_GL_ERROR;
			//
			toSetup = false;
			continue;
		}
		if (m_clickRequest.load(std::memory_order_acquire))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, toDrawToFramebuffer1 ? outputFramebuffer2 : outputFramebuffer1); CHECK_GL_ERROR;
			glReadBuffer(GL_COLOR_ATTACHMENT1); CHECK_GL_ERROR;
			glReadPixels(m_clickRequestPos.x, m_clickRequestPos.y, 1, 1, GL_RGBA_INTEGER, GL_INT, m_clickRequestData.data()); CHECK_GL_ERROR;
			m_clickRequest.store(false, std::memory_order_release);
		}
		if (!m_submitionsDone.load(std::memory_order_acquire))
		{
			continue;
		}
		//Timer<std::chrono::microseconds> timer("Renderer loop");
		const GLuint currentOutputFrameBuffer(toDrawToFramebuffer1 ? outputFramebuffer1 : outputFramebuffer2);
		const GLuint currentOutputTexture(toDrawToFramebuffer1 ? outputTexture1 : outputTexture2);
		const GLuint currentClickingTexture(toDrawToFramebuffer1 ? clickingTexture1 : clickingTexture2);
		glViewport(0, 0, (GLsizei)m_viewportSizes.x, (GLsizei)m_viewportSizes.y); CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, currentOutputFrameBuffer); CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, currentOutputTexture); CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_viewportSizes.x, (GLsizei)m_viewportSizes.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, currentClickingTexture); CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, (GLsizei)m_viewportSizes.x, (GLsizei)m_viewportSizes.y, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, nullptr); CHECK_GL_ERROR;
		glClearBufferfv(GL_COLOR, 0, m_clearColor); CHECK_GL_ERROR;
		glClearBufferiv(GL_COLOR, 1, clickingTextureClearColor); CHECK_GL_ERROR;
		glEnable(GL_BLEND); CHECK_GL_ERROR;
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); CHECK_GL_ERROR;
		std::sort(m_stateExecutionSequence.begin(), m_stateExecutionSequence.end(), RenderStateIndicatorComparator{});
		m_unlitTexturesObjectRenderer.Prepare();
		for (const RenderStateIndicator& indicator : m_stateExecutionSequence)
		{
			for (uint8_t stateType : indicator.RenderStates.GetDenseRef())
			{
			switch (static_cast<RenderStateEnum>(stateType))
			{
				case RenderStateEnum::UnlitTexturedObject:
					m_unlitTexturesObjectRenderer.Render();
					break;
				default:
					DASSERT_E(false);
					return;
				}
			}
		}
		glDisable(GL_BLEND); CHECK_GL_ERROR;
		m_debugRectObjectRenderer.Render();
		glFinish(); CHECK_GL_ERROR;
		m_outputTexture.store(currentOutputTexture, std::memory_order_release);
		m_submitionsDone.store(false, std::memory_order_release);
		m_isRenderingDone.store(true, std::memory_order_release); 
		toDrawToFramebuffer1 = !toDrawToFramebuffer1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); CHECK_GL_ERROR;
	glDeleteTextures(1, &outputTexture1); CHECK_GL_ERROR;
	glDeleteTextures(1, &outputTexture2); CHECK_GL_ERROR;
	glDeleteFramebuffers(1, &outputFramebuffer1); CHECK_GL_ERROR;
	glDeleteFramebuffers(1, &outputFramebuffer2); CHECK_GL_ERROR;
}

bool Renderer::TryReadPixelFromClickingTexture(const DVec2& clickPos, std::array<int, 4>& output)
{
	if (m_viewportSizes.x <= 0 || m_viewportSizes.y <= 0 || clickPos.x < 0 || clickPos.y < 0)
	{
		return false;
	}
	m_clickRequestPos = clickPos;
	m_clickRequest.store(true, std::memory_order_release);
	while (m_clickRequest.load(std::memory_order_acquire));
	output = m_clickRequestData;
	return true;
}

void Renderer::FlushBuffers()
{
	m_unlitTexturesObjectRenderer.Flush();
	m_debugRectObjectRenderer.Flush();
	m_stateExecutionSequence.clear();
	m_addedDrawOrders.Clear();
}

}
