#include "ReadWriteLockGuard.h"



namespace DCore
{

ReadWriteLockGuard::ReadWriteLockGuard(LockType desiredLock, LockData& lockData)
	:
	m_thisThreadId(std::this_thread::get_id()),
	m_desiredLock(desiredLock),
	m_lockData(lockData),
	m_wasReading(false),
	m_toFreeLock(false)
{
	HandleLock();
}

ReadWriteLockGuard::~ReadWriteLockGuard()
{
	using lockGuardType = std::lock_guard<std::mutex>;
	if (!m_toFreeLock)
	{
		return;
	}
	{
		lockGuardType guard(m_lockData.Mutex);
		if (m_desiredLock == LockType::ReadLock)
		{
			m_lockData.ReadingThreads.erase(m_thisThreadId);
		}
		else
		{
			m_lockData.IsThreadWriting = false;
			if (m_wasReading)
			{
				m_lockData.ReadingThreads.insert(m_thisThreadId);
			}
		}
	}
	m_lockData.ConditionVariable.notify_all();
}

void ReadWriteLockGuard::HandleLock()
{
	using lockGuardType = std::lock_guard<std::mutex>;
	{
		lockGuardType guard(m_lockData.Mutex);
		switch (m_desiredLock)
		{
		case DCore::ReadLock:
			if (m_lockData.ReadingThreads.count(m_thisThreadId) > 0)
			{
				return;
			}
			m_lockData.Queue.push(m_thisThreadId);
			break;
		case DCore::WriteLock:
			if (m_lockData.IsThreadWriting && m_lockData.WritingThread == m_thisThreadId)
			{
				return;
			}
			if (m_lockData.ReadingThreads.count(m_thisThreadId) > 0)
			{
				m_lockData.ReadingThreads.erase(m_thisThreadId);
				m_lockData.PriorityQueue.push(m_thisThreadId);
				m_wasReading = true;
				break;
			}
			m_lockData.Queue.push(m_thisThreadId);
			break;
		default:
			break;
		}
	}
	m_toFreeLock = true;
	switch (m_desiredLock)
	{
	case LockType::ReadLock:
	{
		uniqueLockType lock(m_lockData.Mutex);
		m_lockData.ConditionVariable.wait(
			lock,
			[&]() -> bool
			{
				return ((m_lockData.PriorityQueue.empty() && m_lockData.Queue.front() == m_thisThreadId) &&
						!m_lockData.IsThreadWriting);
			});
		m_lockData.ReadingThreads.insert(m_thisThreadId);
		Notify(lock);
		return;
	}
	case LockType::WriteLock:
	{
		uniqueLockType lock(m_lockData.Mutex);
		m_lockData.ConditionVariable.wait(
			lock,
			[&]() -> bool
			{
				return ((m_wasReading && m_lockData.PriorityQueue.front() == m_thisThreadId) ||
					(m_lockData.PriorityQueue.empty() && m_lockData.Queue.front() == m_thisThreadId)) &&
					(m_lockData.ReadingThreads.empty() && !m_lockData.IsThreadWriting);
			});
		m_lockData.WritingThread = m_thisThreadId;
		m_lockData.IsThreadWriting = true;
		Notify(lock);
		return;
	}
	}
}

void ReadWriteLockGuard::Notify(uniqueLockType& lock)
{
	if (m_wasReading)
	{
		m_lockData.PriorityQueue.pop();
	}
	else
	{
		m_lockData.Queue.pop();
	}
	lock.unlock();
	m_lockData.ConditionVariable.notify_all();
}

}
