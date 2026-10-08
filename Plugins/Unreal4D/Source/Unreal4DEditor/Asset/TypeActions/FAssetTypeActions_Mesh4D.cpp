#include "FAssetTypeActions_Mesh4D.h"

#include "../../../Unreal4DCore/Model/Mesh/Base/Mesh4D.h"

#include "EditorFramework/AssetImportData.h"

UClass* FAssetTypeActions_Mesh4D::GetSupportedClass() const
{
	return UMesh4D::StaticClass();
}
