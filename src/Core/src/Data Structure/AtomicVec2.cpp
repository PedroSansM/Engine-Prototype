#include "AtomicVec2.h"



namespace DCore
{

AtomicVec2::AtomicVec2()
	:
	m_x(0),
	m_y(0)
{}

AtomicVec2::AtomicVec2(DFloat value)
	:
	m_x(value),
	m_y(value)
{}

AtomicVec2::AtomicVec2(DFloat x, DFloat y)
	:
	m_x(x),
	m_y(y)
{}

AtomicVec2::AtomicVec2(AtomicVec2& other)
{
	DVec2 values(other.Get());
	m_x = values.x;
	m_y = values.y;
}

void AtomicVec2::Set(DFloat x, DFloat y)
{	
	std::lock_guard<std::mutex> guard(m_mutex);
	m_x = x;
	m_y = y;
}

DVec2 AtomicVec2::Get()
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return {m_x, m_y};
}

float AtomicVec2::GetX()
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return m_x;
}

float AtomicVec2::GetY()
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return m_y;
}

}
