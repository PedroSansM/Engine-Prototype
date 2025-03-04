#pragma once

#include "AsyncOperation.h"



namespace DCore
{

template <class ReturnType>
class Promise : public AsyncOperation
{
public:
	Promise() = default;
	virtual ~Promise() = default;
public:
	virtual ReturnType Get()
	{
		Wait();
		return m_returnType;
	}
public:
	virtual void Begin() = 0;
protected:
	ReturnType m_returnType;
};

}
