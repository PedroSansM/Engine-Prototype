#pragma once

#include "DommusCore.h"

#include <filesystem>



namespace DEditor
{

class ProgramContext
{
public:
	using pathType = std::filesystem::path;
public:
	~ProgramContext() = default;
public:
	static ProgramContext& Get()
	{
		static ProgramContext programContext;
		return programContext;
	}
public:
	void SetProjectAssetsDirectoryPath(const char* path)
	{
		m_assetsDirectoryPath = path;
		m_projectRootDirectoryPath = m_assetsDirectoryPath.parent_path();
	}

	const pathType& GetProjectAssetsDirectoryPath() const
	{
		return m_assetsDirectoryPath;
	}

	const pathType& GetProjectRootDirectoryPath() const
	{
		return m_projectRootDirectoryPath;
	}

	const pathType GetSoundBacksDirectoryPath() const
	{
		return m_assetsDirectoryPath / "bank";
	}

	void SetEditorAssetsDirectoryPath(const char* path)
	{
		m_editorAssetsDirectoryPath = path;
	}

	const pathType& GetEditorAssetsDirectoryPath() const
	{
		return m_editorAssetsDirectoryPath;
	}
	
	void SetMainContext(GLFWwindow* context)
	{
		m_mainContext = context;
	}

	GLFWwindow* GetMainContext()
	{
		DASSERT_E(m_mainContext != nullptr);
		return m_mainContext;
	}
private:
	ProgramContext()
		:
		m_mainContext(nullptr)
	{}
private:
	pathType m_assetsDirectoryPath;
	pathType m_editorAssetsDirectoryPath;
	pathType m_projectRootDirectoryPath;
	GLFWwindow* m_mainContext;
};

}
