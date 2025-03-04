#pragma once



#include "AsyncOperation.h"
namespace DCore
{

class AsyncOperationRef
{
public:
	AsyncOperationRef(AsyncOperation*);
	AsyncOperationRef(AsyncOperationRef&&);
	~AsyncOperationRef();
public:
	void Wait();
	void Begin();
private:
	AsyncOperation* m_asyncOperation;
};

}
