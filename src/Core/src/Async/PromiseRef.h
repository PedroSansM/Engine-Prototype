#pragma once

#include "Promise.h"

#include <cassert>



namespace DCore 
{

template <class ReturnType>
class PromiseRef
{
public:
	PromiseRef(Promise<ReturnType>* promise)
		:
		m_promise(promise)
	{}

	PromiseRef(PromiseRef<ReturnType>&& other)
		:
		m_promise(other.m_promise)
	{
		other.m_promise = nullptr;
	}

	~PromiseRef()
	{
		delete m_promise;
		m_promise = nullptr;
	}
public:
	ReturnType Get()
	{
		assert(m_promise != nullptr);
		m_promise->Get();
	}

	void Begin()
	{
		assert(m_promise != nullptr);
		m_promise->Begin();
	}
private:
	Promise<ReturnType>* m_promise;
};

}
