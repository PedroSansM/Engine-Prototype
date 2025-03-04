#pragma once

#include <cstddef>



namespace DCore
{

class TextureUnits
{
public:
	static constexpr size_t numberOfTextureUnits{16};
	static constexpr int textureUnits[numberOfTextureUnits]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
public:
	TextureUnits(const TextureUnits&) = delete;
	TextureUnits(TextureUnits&&) = delete;
	~TextureUnits() = default;
private:
	TextureUnits() = default;
};

}
