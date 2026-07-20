#pragma once

#include "Mesh4DActor.h"

class UOrthoplexTetraMesh4D;

#include "Orthoplex4DActor.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Actor|Mesh|Orthoplex")
class AOrthoplex4DActor : public AMesh4DActor
{
	GENERATED_BODY()

private:
	UOrthoplexTetraMesh4D* _orthoplexMesh = nullptr;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh 4D")
	FVector4 OrthoplexSize = FVector4(100.0, 100.0, 100.0, 100.0);

	AOrthoplex4DActor(const FObjectInitializer& Init);

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
