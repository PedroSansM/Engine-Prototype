#include "SceneLoader.h"
#include "DCoreAssert.h"



namespace DCore
{

SceneRef SceneLoader::LoadScene(const stringType& sceneName)
{
	DASSERT_E(m_loadSceneFunc);
	return m_loadSceneFunc(sceneName);
}

}