#pragma once

#include <filesystem>



namespace DEditor
{

class FileBrowser
{
public:
	using pathType = std::filesystem::path;	
public:
	~FileBrowser() = default;
public:
	static FileBrowser& Get()
	{
		static FileBrowser fileSystem;
		return fileSystem;
	}
public:
	bool BrowseFile(const char* fileBrowserTitle, pathType& outPath);
	bool SaveFile(pathType& out);
private:
	FileBrowser() = default;
}; 

}
