#pragma once

#include "../Unreal4DEditorGizmoDefines.h"

#include "Components/PrimitiveComponent.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveViewRelevance.h"

#include "EditorTransformGizmoBaseMesh4D.generated.h"

// Inspired by Unreal's UGizmoBaseComponent class.
// Due to limitations in that class, we have to implement our own base mesh component.
UCLASS(MinimalAPI)
class UEditorTransformGizmoBaseMesh4D : public UPrimitiveComponent
{
	GENERATED_BODY()

protected:
	FLinearColor PrimaryNormalColor = FLinearColor::Red;
	FLinearColor PrimaryHighlightColor = FLinearColor::Red;
	FVector PrimaryTargetPosition = FVector();
	float DepthBias = 0.0f;
	bool bIsHighlighted = false;

public:
	void SetPrimaryColors(const FLinearColor& InPrimaryNormalColor, const FLinearColor& InPrimaryHighlightColor);
	FORCEINLINE FLinearColor GetCurrentPrimaryColor() const { return bIsHighlighted ? PrimaryHighlightColor : PrimaryNormalColor; }
	void SetHighlighted(const bool bInIsHighlighted);

	void SetPrimaryTargetPosition(const FVector& InPrimaryPosition);
	FORCEINLINE FVector GetPrimaryTargetPosition() const { return PrimaryTargetPosition; }

	FORCEINLINE void SetDepthBias(const float InDepthBias) { DepthBias = InDepthBias; }
	FORCEINLINE float GetDepthBias() const { return DepthBias; }

	UEditorTransformGizmoBaseMesh4D();
};

// This proxy class is required for integration with Unreal's threaded rendering system.
class FEditorTransformGizmoBaseMeshSceneProxy4D : public FPrimitiveSceneProxy
{
public:
	static constexpr float LINE_THICKNESS = 10.0f;
	FLinearColor PrimaryColor;
	FVector PrimaryTargetPosition;
	float DepthBias = 0.0f;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* InView) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(InView);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = false;
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(InView);
		return Result;
	}

	virtual bool CanBeOccluded() const override { return false; }

	FEditorTransformGizmoBaseMeshSceneProxy4D(const UPrimitiveComponent* InComponent) : FPrimitiveSceneProxy(InComponent) {}
};
