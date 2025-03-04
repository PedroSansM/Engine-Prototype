#include "AsyncOperationRef.h"

#include <cassert>



namespace DCore 
{

AsyncOperationRef::AsyncOperationRef(AsyncOperation* asyncOperation)
	:
	m_asyncOperation(asyncOperation)
{}

AsyncOperationRef::AsyncOperationRef(AsyncOperationRef&& other)
	:
	m_asyncOperation(other.m_asyncOperation)
{
	other.m_asyncOperation = nullptr;
}

AsyncOperationRef::~AsyncOperationRef()
{
	delete m_asyncOperation;
	m_asyncOperation = nullptr;
}

void AsyncOperationRef::Wait()
{
	assert(m_asyncOperation != nullptr);
	m_asyncOperation->Wait();
}

void AsyncOperationRef::Begin()
{
	assert(m_asyncOperation != nullptr);
	m_asyncOperation->Begin();
}

}
