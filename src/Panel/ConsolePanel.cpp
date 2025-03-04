#include "ConsolePanel.h"

#include "imgui.h"

#include <chrono>



namespace DEditor 
{

// Message
ConsolePanel::Message::Message(const DCore::DString& message, LogLevel logLevel)
	:
	m_message(message),
	m_logLevel(logLevel)
{}
// End Message

// Console Panel
ConsolePanel::ConsolePanel() 
	: 
	m_isOpened(true),
	m_scrollLocked(true)
{}

void ConsolePanel::Render() 
{
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, m_lockData);
	if (!m_isOpened)
	{
		return;
	}
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_HorizontalScrollbar);
	if (!ImGui::Begin("Console", &m_isOpened, windowFlags)) 
	{
		ImGui::End();
		return;
	}
	const ImVec2 regionAvailable(ImGui::GetContentRegionAvail());
	if (ImGui::BeginChild("Functions", {regionAvailable.x, 20.0f}, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (ImGui::Button("Clear"))
		{
			m_messages.clear();
		}
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Lock Scroll");
		ImGui::SameLine();
		ImGui::Checkbox("##LockScroll", &m_scrollLocked);
	}
	ImGui::EndChild();
	if (ImGui::BeginChild("Messages", {regionAvailable.x, 0.0f}, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
	{
		for (const Message& message : m_messages)
		{
			switch (message.GetLogLevel())
			{
				case LogLevel::Default:
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", message.GetMessage().Data());
					break;
				case LogLevel::Warning:
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", message.GetMessage().Data());
					break;
				case LogLevel::Error:
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", message.GetMessage().Data());
					break;
			}
		}
	}
	if (m_scrollLocked)
	{
		ImGui::SetScrollHereY(1.0f);
	}
	ImGui::EndChild();
	ImGui::End();
}

void ConsolePanel::LogMessage(const DCore::DString& message, LogLevel logLevel)
{
	DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
	DCore::DString messageToLog;
	// TODO: Get time into messageToLog
	switch (logLevel) 
	{
	case LogLevel::Default:
		messageToLog.Append("[MESSAGE]  ");
		break;
	case LogLevel::Warning:
		messageToLog.Append("[WARNING]  ");
		break;
	case LogLevel::Error:
		messageToLog.Append("[ERROR]  ");
		break;
	}
	messageToLog.Append(message);
	m_messages.push_back(Message(messageToLog, logLevel));
}
// End Console Panel

}
