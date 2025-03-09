#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include <thread>
#include <queue>



namespace DCore
{

enum LockType : uint8_t
{
	ReadLock,
	WriteLock,
};

using LockData = struct LockData
{
	LockData()
		:
		IsThreadWriting(false)
	{}

	std::unordered_set<std::thread::id> ReadingThreads;
	std::thread::id WritingThread;
	bool IsThreadWriting;
	std::queue<std::thread::id> Queue;
	std::unordered_set<std::thread::id> ThreadsInQueue;
	std::mutex Mutex;
};

class ReadWriteLockGuard
{
public:
	using threadIdType = std::thread::id;
public:
	ReadWriteLockGuard(LockType desiredLock, LockData&);
	~ReadWriteLockGuard();
public:
	template <class Lockable>
	ReadWriteLockGuard(LockType desiredLock, Lockable& lockable)
		:
		m_thisThreadId(std::this_thread::get_id()),
		m_desiredLock(desiredLock),
		m_lockData(lockable.GetLockData()),
		m_lockObtained(false)
	{
		HandleLock();
	}
private:
	threadIdType m_thisThreadId;
	LockType m_desiredLock;
	LockData& m_lockData;
	bool m_lockObtained;
private:
	void HandleLock();
};

}
