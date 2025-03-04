#pragma once

#include <filesystem>



namespace DEditor
{

class Path
{
public:
	~Path() = default;
public:
	static Path& Get()
	{
		static Path path;
		return path;
	}
public:
	std::filesystem::path MakePathRelativeToAssetsDirectory(const std::filesystem::path& path) const;
private:
	Path() = default;
};

}
