#include "Orthoplex4DActor.h"

#include "../../Model/Mesh/Tetra/OrthoplexTetraMesh4D.h"

AOrthoplex4DActor::AOrthoplex4DActor(const FObjectInitializer& Init) : Super(Init)
{
	_orthoplexMesh = CreateDefaultSubobject<UOrthoplexTetraMesh4D>(TEXT("OrthoplexTetraMesh4D"));
	_orthoplexMesh->SetSize(OrthoplexSize);
	Mesh4DComponent->Mesh4D = _orthoplexMesh;
}

#if WITH_EDITOR
void AOrthoplex4DActor::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
	Super::PostEditChangeProperty(InPropertyChangedEvent);
	_orthoplexMesh->SetSize(OrthoplexSize);
}
#endif // WITH_EDITOR
