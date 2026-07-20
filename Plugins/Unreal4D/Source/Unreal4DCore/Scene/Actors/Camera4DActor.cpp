#include "Camera4DActor.h"

ACamera4DActor::ACamera4DActor(const FObjectInitializer& Init) : Super(Init)
{
	Camera4DComponent = CreateDefaultSubobject<UCamera4DComponent>(TEXT("Camera4DComponent"));
	Camera4DComponent->SetupAttachment(RootComponent);
	Camera4DComponent->SetRelativeTransform(FTransform::Identity);
}
