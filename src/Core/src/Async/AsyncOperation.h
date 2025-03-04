#pragma once

#include <thread>



namespace DCore
{

class AsyncOperation
{
public:
	AsyncOperation() = default ;
	virtual ~AsyncOperation() = default;
public:
	virtual void Wait();
public:
	virtual void Begin() = 0;
protected:
	std::thread m_thread;
};

}
