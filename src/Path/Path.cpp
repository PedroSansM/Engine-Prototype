#include "Path.h"

#include <vector>



namespace DEditor 
{

std::filesystem::path Path::MakePathRelativeToAssetsDirectory(const std::filesystem::path& path) const
{
	std::filesystem::path relativeToAssetsDirectory(path);
	std::vector<std::string> pathElements;
	while (relativeToAssetsDirectory.filename() != "assets")
	{
		pathElements.insert(pathElements.begin(), relativeToAssetsDirectory.filename().string());
		relativeToAssetsDirectory = relativeToAssetsDirectory.parent_path();
	}
	relativeToAssetsDirectory.clear();
	for (const std::string& element : pathElements)
	{
		if (relativeToAssetsDirectory.empty())
		{
			relativeToAssetsDirectory = element;
			continue;
		}
		relativeToAssetsDirectory /= element;
	}
	return relativeToAssetsDirectory;
}

}
