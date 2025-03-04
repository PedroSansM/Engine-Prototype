#include "AsyncOperation.h"



namespace DCore
{

void AsyncOperation::Wait()
{
	m_thread.join();
}

}

