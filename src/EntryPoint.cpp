#include "Panels.h"
#include "ProgramContext.h"
#include "Log.h"
#include "GlobalConfigurationSerializer.h"
#include "Window.h"

#include "DommusCore.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"

#include <cstdlib>
#include <iostream>
#include <stdlib.h>




void GLFWErrorCallback(int code, const char* error)
{
	std::cout << "GLFW: " << error << std::endl;
}

void GLFWDropCallback(GLFWwindow* window, int count, const char** paths)
{
	DEditor::Window::Get().DispatchDragAndDropEvent(count, paths);
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		std::cout << "Fail to init COM" << std::endl;
		return EXIT_FAILURE;
	}
#endif
	DASSERT_E(argc == 3);
	DEditor::ProgramContext::Get().SetProjectAssetsDirectoryPath(argv[1]);
	DEditor::ProgramContext::Get().SetEditorAssetsDirectoryPath(argv[2]);
	DEditor::GlobalConfigurationSerializer::Get();
	glfwSetErrorCallback(GLFWErrorCallback);
	if (!glfwInit())
	{
		return EXIT_FAILURE;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window(glfwCreateWindow(800, 600, "Dommus Editor", nullptr, nullptr));
	glfwMakeContextCurrent(window);
	glfwSetDropCallback(window, GLFWDropCallback);
	glfwSwapInterval(0);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	DEditor::ProgramContext::Get().SetMainContext(window);
	DCore::ReturnError error(DCore::Shaders::Get().TryCompileShaders());
	if (!error.Ok)
	{
		DEditor::Log::Get().TerminalLog("Fail to compile shaders\nError message: ", error.Message.Data());
		DASSERT_E(false);
		return EXIT_FAILURE;
	}
	// ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io(ImGui::GetIO()); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
	DCore::Input::Get().SetUserCallback(ImGui_ImplGlfw_KeyCallback);
	ImGuizmo::Enable(true);
	//
	while (!glfwWindowShouldClose(window))
	{
		//DCore::Timer<std::chrono::microseconds> timer("Main thread loop");
		int width(0), height(0);
		glfwPollEvents(); 
		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		ImGuizmo::BeginFrame();
		ImGui::DockSpaceOverViewport();
		DEditor::Panels::Get().RenderPanels();
//	ImGui::ShowDemoWindow();
		ImGui::Render();
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	return EXIT_SUCCESS;
}
