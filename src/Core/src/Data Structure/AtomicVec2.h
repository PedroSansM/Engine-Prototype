#pragma once

#include "SerializationTypes.h"

#include <mutex>



namespace DCore
{

class AtomicVec2
{
public:
	AtomicVec2();
	AtomicVec2(DFloat);
	AtomicVec2(DFloat x, DFloat y);
	AtomicVec2(AtomicVec2& other);
	~AtomicVec2() = default;
public:
	void Set(DFloat x, DFloat y);
	DVec2 Get();
private:
	DFloat m_x, m_y;
	std::mutex m_mutex;
};

}
