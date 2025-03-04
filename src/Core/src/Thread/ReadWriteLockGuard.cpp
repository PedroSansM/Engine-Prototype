#include "ReadWriteLockGuard.h"
#include "DCoreAssert.h"

#include <iostream>



namespace DCore
{

ReadWriteLockGuard::ReadWriteLockGuard(LockType desiredLock, LockData& lockData)
	:
	m_thisThreadId(std::this_thread::get_id()),
	m_desiredLock(desiredLock),
	m_lockData(lockData),
	m_wasReading(false)
{
	HandleLock();
}

ReadWriteLockGuard::~ReadWriteLockGuard()
{
	using lockGuardType = std::lock_guard<std::mutex>;
	lockGuardType guard(m_lockData.Mutex);
	if (m_desiredLock == LockType::ReadLock)
	{
		m_lockData.ReadingThreads.erase(m_thisThreadId);
		return;
	}
	m_lockData.IsThreadWriting = false;
	if (m_wasReading)
	{
		m_lockData.ReadingThreads.insert(m_thisThreadId);
	}
}

void ReadWriteLockGuard::HandleLock()
{
	using lockGuardType = std::lock_guard<std::mutex>;
	{
		lockGuardType guard(m_lockData.Mutex);
		if (m_desiredLock == LockType::WriteLock && m_lockData.ReadingThreads.count(m_thisThreadId) != 0)
		{
			m_lockData.ReadingThreads.erase(m_thisThreadId);
			m_lockData.PriorityQueue.push_back(m_thisThreadId);
			m_wasReading = true;
		}
		else
		{
			m_lockData.Queue.push_back(m_thisThreadId);
		}
		if (m_desiredLock == LockType::WriteLock && m_lockData.IsThreadWriting && m_lockData.WritingThread == m_thisThreadId)
		{
			DASSERT_E(false);
		}
	}
	switch (m_desiredLock)
	{
	case LockType::ReadLock:
		while (true)
		{
			lockGuardType guard(m_lockData.Mutex);
			if (((m_wasReading && m_lockData.PriorityQueue.front() == m_thisThreadId) || 
				(m_lockData.PriorityQueue.empty() && m_lockData.Queue.front() == m_thisThreadId)) &&
				!m_lockData.IsThreadWriting)
			{
				m_lockData.ReadingThreads.insert(m_thisThreadId);
				goto End;
			}
		}
		break;
	case LockType::WriteLock:
		while (true)
		{
			lockGuardType guard(m_lockData.Mutex);
			if (((m_wasReading && m_lockData.PriorityQueue.front() == m_thisThreadId) || 
				(m_lockData.PriorityQueue.empty() && m_lockData.Queue.front() == m_thisThreadId)) && 
				(m_lockData.ReadingThreads.size() == 0 && !m_lockData.IsThreadWriting))
			{
				m_lockData.WritingThread = m_thisThreadId;
				m_lockData.IsThreadWriting = true;
				goto End;
			}
		}
		break;
	}
End:
	lockGuardType lock(m_lockData.Mutex);
	if (m_wasReading)
	{
		m_lockData.PriorityQueue.erase(m_lockData.PriorityQueue.begin());
		return;
	}
	m_lockData.Queue.erase(m_lockData.Queue.begin());
}

}
