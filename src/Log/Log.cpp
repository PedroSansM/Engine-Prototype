#include "Log.h"

#include "DommusCore.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>



namespace DEditor
{

void Log::ConsoleLog(LogLevel logLevel, const char* fmt, ...)
{
	static DCore::DString buffer;
	std::va_list args;
	va_start(args, fmt);
	vsprintf(buffer.Data(), fmt, args);
	va_end(args);
	ConsolePanel::Get().LogMessage(buffer, logLevel);
}

#ifdef DEBUG
void Log::TerminalLog(const char* fmt, ...)
{
	DCore::DString buffer;
	std::va_list args;
	va_start(args, fmt);
	vsprintf(buffer.Data(), fmt, args);
	va_end(args);
	std::cout << buffer.Data() << std::endl;
}
#endif

}
