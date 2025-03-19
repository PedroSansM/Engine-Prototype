#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include <thread>
#include <queue>
#include <condition_variable>



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
	std::queue<std::thread::id> PriorityQueue;
	std::mutex Mutex;
	std::condition_variable ConditionVariable;
};

class ReadWriteLockGuard
{
public:
	using threadIdType = std::thread::id;
	using uniqueLockType = std::unique_lock<std::mutex>;
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
		m_wasReading(false),
		m_toFreeLock(false)
	{
		HandleLock();
	}
private:
	threadIdType m_thisThreadId;
	LockType m_desiredLock;
	LockData& m_lockData;
	bool m_wasReading;
	bool m_toFreeLock;
private:
	void HandleLock();
	void Notify(uniqueLockType&);
};

}
