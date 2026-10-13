#include "RenderingSettings4D.h"

#include "Materials/MaterialInterface.h"

#if WITH_EDITOR
#include "../Scene/Components/Mesh4DProjectionComponent.h"
#include "UObject/UObjectIterator.h"
#endif

URenderingSettings4D::URenderingSettings4D()
{
	CategoryName = TEXT("Plugins");
	DefaultProjectionMaterial = FSoftObjectPath(TEXT("/Unreal4D/Materials/MAT_Mesh4DProjection_Default.MAT_Mesh4DProjection_Default"));
}

void URenderingSettings4D::PostInitProperties()
{
	Super::PostInitProperties();
	// Load after config overrides are applied, retaining the asset for GC and cook startup discovery.
	GetDefaultProjectionMaterial();
}

URenderingSettings4D* URenderingSettings4D::Get()
{
	return GetMutableDefault<URenderingSettings4D>();
}

UMaterialInterface* URenderingSettings4D::GetDefaultProjectionMaterial()
{
	LoadedProjectionMaterial = DefaultProjectionMaterial.LoadSynchronous();
	return LoadedProjectionMaterial;
}

#if WITH_EDITOR
void URenderingSettings4D::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URenderingSettings4D, DefaultProjectionMaterial))
	{
		GetDefaultProjectionMaterial();
		for (TObjectIterator<UMesh4DProjectionComponent> Component; Component; ++Component)
		{
			if (Component->IsRegistered())
			{
				Component->MarkRenderStateDirty();
			}
		}
	}
}
#endif
