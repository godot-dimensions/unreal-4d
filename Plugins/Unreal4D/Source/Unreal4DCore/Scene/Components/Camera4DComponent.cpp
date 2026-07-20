#include "Camera4DComponent.h"

#include "../../Math/VectorFuncs4D.h"
#include "../../Render/RenderingServer4D.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

void UCamera4DComponent::OnRegister()
{
	Super::OnRegister();
	URenderingServer4D* renderingServer4D = GetWorld()->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D != nullptr)
	{
		renderingServer4D->RegisterCamera(this);
	}
}

double UCamera4DComponent::GetFocalLength() const
{
	return FMath::Tan((PI - FieldOfView) * 0.5);
}

void UCamera4DComponent::SetFocalLength(const double InFocalLength)
{
	FieldOfView = (float)(PI - 2.0 * FMath::Atan(InFocalLength));
}

FTransform4D UCamera4DComponent::GetGlobalTransform4D() const
{
	UScene4DComponent* parent4DComponent = Cast<UScene4DComponent>(GetAttachParent());
	if (parent4DComponent == nullptr)
	{
		return FTransform4D::Identity();
	}
	return parent4DComponent->GetGlobalTransform4D();
}

bool UCamera4DComponent::IsCurrent() const
{
	return _isCurrent;
}

void UCamera4DComponent::SetCurrent(bool bEnable)
{
	_isCurrent = bEnable;
	URenderingServer4D* renderingServer4D = GetWorld()->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D != nullptr)
	{
		if (bEnable)
		{
			renderingServer4D->MakeCameraCurrent(this);
		}
		else
		{
			renderingServer4D->ClearCameraCurrent(this);
		}
	}
}

void UCamera4DComponent::ClearCurrent(bool bEnableNext)
{
	_isCurrent = false;
	if (bEnableNext)
	{
		URenderingServer4D* renderingServer4D = GetWorld()->GetSubsystem<URenderingServer4D>();
		if (renderingServer4D != nullptr)
		{
			renderingServer4D->ClearCameraCurrent(this);
		}
	}
}

void UCamera4DComponent::MakeCurrent()
{
	_isCurrent = true;
	URenderingServer4D* renderingServer4D = GetWorld()->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D != nullptr)
	{
		renderingServer4D->MakeCameraCurrent(this);
	}
}

// Projection functions.
// TODO: The viewport space and world space conventions will likely need to be adjusted for Unreal Engine's coordinate system.
// Wait to convert these until there's an actual use case for these functions, then we can test and verify correctness.

bool UCamera4DComponent::IsPositionBehind(const FVector4& InGlobalPosition) const
{
	const FTransform4D globalTransform = GetGlobalTransform4D();
	const FVector4 globalBasisZ = FVector4(globalTransform.Basis.M[2][0], globalTransform.Basis.M[2][1], globalTransform.Basis.M[2][2], globalTransform.Basis.M[2][3]);
	return Dot4(globalBasisZ, InGlobalPosition - globalTransform.Origin) > -GNearClippingPlane;
}

FVector4 UCamera4DComponent::ViewportToWorldRayOrigin(const FVector2D& InViewportPosition) const
{
	const FTransform4D globalTransform = GetGlobalTransform4D();
	// Perspective cameras always have their ray origin at the camera's position.
	if (ProjectionMode == ECameraProjectionMode::Type::Perspective)
	{
		return globalTransform.Origin;
	}
	// Orthographic cameras have ray origins offset by the orthographic size.
	FVector2D viewportSize;
	GEngine->GameViewport->GetViewportSize(viewportSize);
	// In Unreal, orthographic cameras always keep width (but perspective cameras use height).
	const double pixelSize = viewportSize.X;
	const FVector2D ScaledPosition = (InViewportPosition * 2.0f - viewportSize) * (OrthoWidth / pixelSize);
	const FVector4 globalBasisX = FVector4(globalTransform.Basis.M[0][0], globalTransform.Basis.M[0][1], globalTransform.Basis.M[0][2], globalTransform.Basis.M[0][3]);
	const FVector4 globalBasisY = FVector4(globalTransform.Basis.M[1][0], globalTransform.Basis.M[1][1], globalTransform.Basis.M[1][2], globalTransform.Basis.M[1][3]);
	return globalTransform.Origin +
		globalBasisX * ScaledPosition.X +
		globalBasisY * ScaledPosition.Y;
}

FVector4 UCamera4DComponent::ViewportToWorldRayDirection(const FVector2D& InViewportPosition) const
{
	const FTransform4D globalTransform = GetGlobalTransform4D();
	// Orthographic cameras always have their ray direction pointing straight forward.
	if (ProjectionMode == ECameraProjectionMode::Type::Orthographic)
	{
		return FVector4(-globalTransform.Basis.M[2][0],
		                -globalTransform.Basis.M[2][1],
		                -globalTransform.Basis.M[2][2],
		                -globalTransform.Basis.M[2][3]);
	}
	// Perspective cameras have ray directions pointing more to the side when near the sides of the viewport.
	FVector2D viewportSize;
	GEngine->GameViewport->GetViewportSize(viewportSize);
	// In Unreal, perspective cameras always keep height (but orthographic cameras use width).
	const double pixelSize = viewportSize.Y;
	const double focalLength = GetFocalLength();
	const FVector2D scaledPosition = (InViewportPosition * 2.0f - viewportSize) / pixelSize;
	const FVector4 globalBasisX = FVector4(globalTransform.Basis.M[0][0], globalTransform.Basis.M[0][1], globalTransform.Basis.M[0][2], globalTransform.Basis.M[0][3]);
	const FVector4 globalBasisY = FVector4(globalTransform.Basis.M[1][0], globalTransform.Basis.M[1][1], globalTransform.Basis.M[1][2], globalTransform.Basis.M[1][3]);
	const FVector4 globalBasisZ = FVector4(globalTransform.Basis.M[2][0], globalTransform.Basis.M[2][1], globalTransform.Basis.M[2][2], globalTransform.Basis.M[2][3]);
	const FVector4 rayDirection =
		globalBasisX * scaledPosition.X +
		globalBasisY * -scaledPosition.Y +
		globalBasisZ * -focalLength;
	return UVectorFuncs4D::Vector4_Normalized(rayDirection);
}

FVector2D UCamera4DComponent::WorldToViewportLocalNormal(const FVector4& InLocalPosition) const
{
	if (ProjectionMode == ECameraProjectionMode::Type::Perspective)
	{
		return FVector2D(InLocalPosition.X, InLocalPosition.Y) * (-1.0 / InLocalPosition.Z);
	}
	return FVector2D(InLocalPosition.X, InLocalPosition.Y) / OrthoWidth;
}

FVector2D UCamera4DComponent::WorldToViewport(const FVector4& InGlobalPosition) const
{
	const FTransform4D globalTransform = GetGlobalTransform4D();
	const FVector4 localPosition = globalTransform.TransposeTransformPosition(InGlobalPosition);
	FVector2D viewportSize;
	GEngine->GameViewport->GetViewportSize(viewportSize);
	// In Unreal, orthographic cameras always keep width, and perspective cameras keep height.
	const double pixelSize = (ProjectionMode == ECameraProjectionMode::Type::Orthographic) ? viewportSize.X : viewportSize.Y;
	const FVector2D projected = WorldToViewportLocalNormal(localPosition);
	return (projected * pixelSize + viewportSize) * 0.5;
}
