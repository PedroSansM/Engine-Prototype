#pragma once

#include "ReciclingVector.h"
#include "Asset.h"



namespace DCore
{

template <class AssetType, class IdType = size_t, class VersionType = size_t>
using AssetContainerType = ReciclingVector<Asset<AssetType>, IdType, VersionType>;

}
