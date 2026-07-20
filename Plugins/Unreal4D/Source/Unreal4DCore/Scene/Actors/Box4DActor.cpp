#include "Box4DActor.h"

#include "../../Model/Mesh/Tetra/BoxTetraMesh4D.h"

ABox4DActor::ABox4DActor(const FObjectInitializer& Init) : Super(Init)
{
	_boxMesh = CreateDefaultSubobject<UBoxTetraMesh4D>(TEXT("BoxTetraMesh4D"));
	_boxMesh->SetSize(BoxSize);
	Mesh4DComponent->Mesh4D = _boxMesh;
}

#if WITH_EDITOR
void ABox4DActor::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
	Super::PostEditChangeProperty(InPropertyChangedEvent);
	_boxMesh->SetSize(BoxSize);
}
#endif // WITH_EDITOR
