#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "../Unreal4DCore/Math/Transform4D.h"

#include "Unreal4DEditorLibrary.generated.h"

UCLASS()
class UNREAL4DEDITOR_API UUnreal4DEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	static void SetEditorCameraFullTransform(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D);
};
