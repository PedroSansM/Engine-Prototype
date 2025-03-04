#pragma once

#include "DommusCore.h"

#include <vector>



namespace DEditor
{

enum class LogLevel
{
	Default,
	Warning,
	Error
};

class ConsolePanel
{
	friend class UtilityPanel;
	friend class Panels;
	friend class Log;
private:
	class Message;
public:
	using lockDataType = DCore::LockData;
	using messageContainerType = std::vector<Message>;
public:
	~ConsolePanel() = default;
private:
	class Message
	{
	public:
		Message(const DCore::DString&, LogLevel);
		~Message() = default;
	public:
		const DCore::DString& GetMessage() const
		{
			return m_message;
		}

		LogLevel GetLogLevel() const
		{
			return m_logLevel;
		}
	private:
		DCore::DString m_message;
		LogLevel m_logLevel;
	};
private:
	static ConsolePanel& Get()
	{
		static ConsolePanel consolePanel;
		return consolePanel;
	}
private:
	void Render();
	void LogMessage(const DCore::DString&, LogLevel);
private:
	void Open()
	{
		m_isOpened = true;
	}
private:
	ConsolePanel();
private:
	bool m_isOpened;
	bool m_scrollLocked;
	messageContainerType m_messages;
	lockDataType m_lockData;
};

}
