#include "FileBrowser.h"

#include <shtypes.h>
#include <shobjidl_core.h>
#include <stringapiset.h>
#include <cstdlib>



namespace DEditor
{

static constexpr size_t bufSize{ 257 };
static wchar_t wbuf[bufSize];
static char buf[bufSize];

bool FileBrowser::BrowseFile(const char* fileBrowserTitle, pathType& outPath)
{
	bool result(false);
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr))
		{
			DWORD dwFlags;
			hr = pfd->GetOptions(&dwFlags);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetTitle(L"Save");
				if (SUCCEEDED(hr))
				{
					hr = pfd->SetOptions(dwFlags | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);
					if (SUCCEEDED(hr))
					{
						hr = pfd->Show(NULL);
						if (SUCCEEDED(hr))
						{
							IShellItem* psiResult;
							hr = pfd->GetResult(&psiResult);
							if (SUCCEEDED(hr))
							{
								PWSTR pszFilePath = NULL;
								hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
								if (SUCCEEDED(hr))
								{
									int conversionResult(
										WideCharToMultiByte(
											CP_ACP,
											0,
											pszFilePath,
											std::wcslen(pszFilePath),
											buf,
											bufSize,
											nullptr,
											nullptr
										)
									);
									result = conversionResult != 0;
									outPath = buf;
									CoTaskMemFree(pszFilePath);
								}
								psiResult->Release();
							}
						}
					}
				}
			}
		}
		pfd->Release();
	}
	return result;
}

bool FileBrowser::SaveFile(pathType& out)
{
	const COMDLG_FILTERSPEC pngFilter({L"Portable Network Graphics (*.png)", L"*.png"});
	bool result(false);
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(
		CLSID_FileSaveDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr))
		{
			DWORD dwFlags;
			hr = pfd->GetOptions(&dwFlags);
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetTitle(L"Save");
				if (SUCCEEDED(hr))
				{
					hr = pfd->SetOptions(dwFlags);
					if (SUCCEEDED(hr))
					{
						hr = pfd->SetFileTypes(1, &pngFilter);
						if (SUCCEEDED(hr))
						{
							hr = pfd->Show(NULL);
							if (SUCCEEDED(hr))
							{
								IShellItem* psiResult;
								hr = pfd->GetResult(&psiResult);
								if (SUCCEEDED(hr))
								{
									PWSTR pszFilePath = NULL;
									hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
									if (SUCCEEDED(hr))
									{
										int conversionResult(
											WideCharToMultiByte(
												CP_ACP,
												0,
												pszFilePath,
												std::wcslen(pszFilePath),
												buf,
												bufSize,
												nullptr,
												nullptr
											)
										);
										result = conversionResult != 0;
										out = buf;
										CoTaskMemFree(pszFilePath);
									}
									psiResult->Release();
								}
							}
						}
					}
				}
			}
		}
		pfd->Release();
	}
	return result;
}

}