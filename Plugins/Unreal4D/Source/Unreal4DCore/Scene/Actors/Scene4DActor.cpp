#include "Scene4DActor.h"

UScene4DComponent* AScene4DActor::GetSelf4DComponent() const
{
	return Root4DComponent;
}

AScene4DActor::AScene4DActor(const FObjectInitializer& Init) : Super(Init)
{
	Root4DComponent = CreateDefaultSubobject<UScene4DComponent>(TEXT("Root4DComponent"));
	SetRootComponent(Root4DComponent);
}
