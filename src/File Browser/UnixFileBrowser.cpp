#include "FileBrowser.h"

#include <cstring>



namespace DEditor
{

bool FileBrowser::BrowseFile(const char* fileBrowserTitle, std::filesystem::path& outPath)
{
	static constexpr size_t pathSize(512);
	char buf[pathSize];
	std::memset(buf, 0, pathSize);
	char command[pathSize];
	std::strcpy(command, "zenity --file-selection --title=");
	std::strcat(command, fileBrowserTitle);
	FILE* f(popen(command, "r"));
	if (f == nullptr)
	{
		return false;
	}
	const char* result(std::fgets(buf, pathSize, f));
	if (pclose(f) == -1)
	{
		return false;
	}
	if (result == nullptr)
	{
		return false;
	}
	if (strlen(buf) == 0)
	{
		return false;
	}
	for (size_t offset(0); offset < pathSize; offset++)
	{
		if (buf[offset] == '\n')
		{
			buf[offset] = '\0';
		}
	}
	outPath = buf;
	return true;
}

bool FileBrowser::SaveFile(pathType& out)
{
	char buf[512];
	char command[256];
	std::memset(buf, '\0', 512);
	std::strcpy(command, "zenity --file-selection --save --title=");
	std::strcat(command, "Save File");
	FILE* f(popen(command, "r"));
	if (f == nullptr)
	{
		return false;
	}
	const char* result(std::fgets(buf, 512, f));
	if (pclose(f) == 1)
	{
		return false;
	}
	if (result == nullptr)
	{
		return false;
	}
	for (size_t offset(0); offset < 512; offset++)
	{
		if (buf[offset] == '\n')
		{
			buf[offset] = '\0';
		}
	}
	out = buf;
	return true;
}

}
