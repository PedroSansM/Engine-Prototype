#pragma once

#include "SerializationTypes.h"

#include <cstdint>



namespace DCore
{

struct Quad2
{
	DVec2 BottomLeft;
	DVec2 BottomRight;
	DVec2 TopRight;
	DVec2 TopLeft;

	const DVec2& At(uint8_t index) const
	{
		switch (index)
		{
		case 0:
			return BottomLeft;
		case 1:
			return BottomRight;
		case 2:
			return TopRight;
		case 3:
			return TopLeft;
		default:
			DASSERT_E(false);
			return TopLeft;
		}
	}
};

struct Quad2WithId
{
	size_t Id;
	Quad2 Quad;
};

struct Quad3
{
	DVec3 BottomLeft;
	DVec3 BottomRight;
	DVec3 TopRight;
	DVec3 TopLeft;

	const DVec3& At(uint8_t index) const
	{
		switch (index)
		{
		case 0:
			return BottomLeft;
		case 1:
			return BottomRight;
		case 2:
			return TopRight;
		case 3:
			return TopLeft;
		default:
			DASSERT_E(false);
			return TopLeft;
		}
	}
};

struct Quad3WithId
{
	size_t Id;
	Quad3 Quad;
};

struct Quad2WithIdComparator
{
	bool operator()(const Quad2WithId& a, const Quad2WithId& b) const
	{
		return a.Id < b.Id;
	}
};

struct Quad3WithIdComparator
{
	bool operator()(const Quad3WithId& a, const Quad3WithId& b) const
	{
		return a.Id < b.Id;
	}
};

}
