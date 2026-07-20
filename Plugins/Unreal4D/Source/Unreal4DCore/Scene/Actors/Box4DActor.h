#pragma once

#include "Mesh4DActor.h"

class UBoxTetraMesh4D;

#include "Box4DActor.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Actor|Mesh|Box")
class ABox4DActor : public AMesh4DActor
{
	GENERATED_BODY()

private:
	UBoxTetraMesh4D* _boxMesh = nullptr;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh 4D")
	FVector4 BoxSize = FVector4(100.0, 100.0, 100.0, 100.0);

	ABox4DActor(const FObjectInitializer& Init);

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
