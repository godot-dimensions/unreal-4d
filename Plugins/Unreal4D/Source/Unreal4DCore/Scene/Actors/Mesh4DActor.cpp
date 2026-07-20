#include "Mesh4DActor.h"

AMesh4DActor::AMesh4DActor(const FObjectInitializer& Init) : Super(Init)
{
	Mesh4DComponent = CreateDefaultSubobject<UMesh4DComponent>(TEXT("Mesh4DComponent"));
	SetRootComponent(Mesh4DComponent);
}

void AMesh4DActor::PostLoad()
{
	Super::PostLoad();

	if (!UpgradedRoot4DComponent)
	{
		if (Root4DComponent)
		{
			const FTransform4D Mesh4DTransform = Root4DComponent->GetGlobalTransform4D().Compose(Mesh4DComponent->GetLocalTransform4D());

			if (Mesh4DComponent->GetAttachParent() == Root4DComponent)
			{
				Mesh4DComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				SetRootComponent(Mesh4DComponent);
			}

			Mesh4DComponent->SetGlobalTransform4D(Mesh4DTransform);
		}

		UpgradedRoot4DComponent = true;
	}

	if (Mesh4DComponentGPU)
	{
		if (!Mesh4DComponent->Mesh4D)
		{
			Mesh4DComponent->Mesh4D = Mesh4DComponentGPU->Mesh4D;
		}

		if (!Mesh4DComponent->Material)
		{
			Mesh4DComponent->Material = Mesh4DComponentGPU->Material;
		}
	}
}
