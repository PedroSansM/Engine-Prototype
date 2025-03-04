#pragma once

#include "ConsolePanel.h"

#ifdef DEBUG
#include <iostream>
#endif



namespace DEditor
{

class Log
{
public:
	~Log() = default;
public:
	static Log& Get()
	{
		static Log log;
		return log;
	}
public:
	void ConsoleLog(LogLevel, const char* fmt, ...);
#ifdef DEBUG
	void TerminalLog(const char* fmt, ...);
#else
	inline void TerminalLog(const char* fmt, ...)
	{}
#endif
private:
	Log() = default;
};

}
