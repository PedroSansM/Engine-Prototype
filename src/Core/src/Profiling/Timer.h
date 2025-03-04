#pragma once

#include <chrono>
#include <iostream>



namespace DCore
{

template <class DurationType>
class DurationPrinter {};

template <>
class DurationPrinter<std::chrono::seconds>
{
public:
	static void Print(const char* scopeName, std::chrono::seconds duration)
	{
		std::cout << "Scope: " << scopeName << "\n\t" << "Time: " << duration.count() << "s" << std::endl;
	}
};

template <>
class DurationPrinter<std::chrono::milliseconds>
{
public:
	static void Print(const char* scopeName, std::chrono::milliseconds duration)
	{
		std::cout << "Scope: " << scopeName << "\n\t" << "Time: " << duration.count() << "ms" << std::endl;
	}
};

template <>
class DurationPrinter<std::chrono::microseconds>
{
public:
	static void Print(const char* scopeName, std::chrono::microseconds duration)
	{
		std::cout << "Scope: " << scopeName << "\n\t" << "Time: " << duration.count() << "us" << std::endl;
	}
};

template <>
class DurationPrinter<std::chrono::nanoseconds>
{
public:
	static void Print(const char* scopeName, std::chrono::nanoseconds duration)
	{
		std::cout << "Scope: " << scopeName << "\n\t" << "Time: " << duration.count() << "ns" << std::endl;
	}
};

template <class DurationType = std::chrono::milliseconds>
class Timer
{
public:
	Timer(const char* name)
		:
		m_name(name),
		m_startTimepoint(std::chrono::high_resolution_clock::now())
	{}
	~Timer()
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> endTimepoint(std::chrono::high_resolution_clock::now());
		DurationType duration(std::chrono::duration_cast<DurationType>(endTimepoint - m_startTimepoint));
		DurationPrinter<DurationType>::Print(m_name, duration);
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
	const char* m_name;
};

}
