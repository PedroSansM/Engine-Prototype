#pragma once

#include "DCoreAssert.h"

#include <cstddef>
#include <cstring>
#include <string>



namespace DCore
{

enum class AspectRatio : size_t
{
	FreeAspect = 0,
	A_16_9 = 1,
};

class AspectRatioUtilities
{
public:
	static constexpr size_t numberOfAspectRatios{2};
public:
	AspectRatioUtilities(const AspectRatioUtilities&) = delete;
	AspectRatioUtilities(AspectRatioUtilities&&) = delete;
	~AspectRatioUtilities() = default;
public:
	static AspectRatioUtilities& Get()
	{
		static AspectRatioUtilities instance;
		return instance;
	}
public:
	const char* GetAspectRatioString(AspectRatio aspectRatio) const
	{
		return m_aspectRatioStrings[static_cast<size_t>(aspectRatio)];
	}

	AspectRatio GetAspectRatioFromString(const char* string)
	{
		for (size_t i(0); i < numberOfAspectRatios; i++)
		{
			if (strcmp(m_aspectRatioStrings[i], string) == 0)
			{
				return static_cast<AspectRatio>(i);
			}
		}
		DASSERT_E(false);
		return AspectRatio::FreeAspect;
	}
	
	float GetAspectRatioValue(AspectRatio aspectRatio) const
	{
		switch (aspectRatio)
		{
		case AspectRatio::FreeAspect:
			return 0.0f;
		case AspectRatio::A_16_9:
			return 1.777778f;
		}
		return 0.0f;
	}
private:
	AspectRatioUtilities()
		:
		m_aspectRatioStrings{"Free Aspect", "16:9"}
	{};	
private:
	const char* m_aspectRatioStrings[numberOfAspectRatios];
};

}
