#include "ReadWriteLockGuard.h"
#include "DCoreAssert.h"



namespace DCore
{

ReadWriteLockGuard::ReadWriteLockGuard(LockType desiredLock, LockData& lockData)
	:
	m_thisThreadId(std::this_thread::get_id()),
	m_desiredLock(desiredLock),
	m_lockData(lockData),
	m_lockObtained(false)
{
	HandleLock();
}

ReadWriteLockGuard::~ReadWriteLockGuard()
{
	using lockGuardType = std::lock_guard<std::mutex>;
	if (!m_lockObtained)
	{
		return;
	}
	lockGuardType guard(m_lockData.Mutex);
	if (m_desiredLock == LockType::ReadLock)
	{
		m_lockData.ReadingThreads.erase(m_thisThreadId);
		return;
	}
	m_lockData.IsThreadWriting = false;
}

void ReadWriteLockGuard::HandleLock()
{
	using lockGuardType = std::lock_guard<std::mutex>;
	{
		lockGuardType guard(m_lockData.Mutex);
		switch (m_desiredLock)
		{
		case DCore::ReadLock:
			if (m_lockData.IsThreadWriting && m_lockData.WritingThread == m_thisThreadId)
			{
				DASSERT_E(false);
			}
			if (m_lockData.ReadingThreads.count(m_thisThreadId) != 0)
			{
				return;
			}
			break;
		case DCore::WriteLock:
			if (m_lockData.ReadingThreads.count(m_thisThreadId) > 0)
			{
				DASSERT_E(false);
			}
			if (m_lockData.IsThreadWriting && m_lockData.WritingThread == m_thisThreadId)
			{
				return;
			}
			break;
		default:
			break;
		}
		m_lockData.Queue.push(m_thisThreadId);
	}
	switch (m_desiredLock)
	{
	case LockType::ReadLock:
		while (true)
		{
			lockGuardType guard(m_lockData.Mutex);
			if (m_lockData.Queue.front() == m_thisThreadId &&
				!m_lockData.IsThreadWriting)
			{
				m_lockData.ReadingThreads.insert(m_thisThreadId);
				m_lockData.Queue.pop();
				m_lockObtained = true;
				return;
			}
		}
		break;
	case LockType::WriteLock:
		while (true)
		{
			lockGuardType guard(m_lockData.Mutex);
			if (m_lockData.Queue.front() == m_thisThreadId &&
				m_lockData.ReadingThreads.size() == 0 && 
				!m_lockData.IsThreadWriting)
			{
				m_lockData.WritingThread = m_thisThreadId;
				m_lockData.IsThreadWriting = true;
				m_lockData.Queue.pop();
				m_lockObtained = true;
				return;
			}
		}
		break;
	}
}

}
