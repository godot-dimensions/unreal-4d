#pragma once

#include "EditorTransformGizmoBaseMesh4D.h"

#include "EditorTransformGizmoBiplaneMesh4D.generated.h"

// Inspired by Unreal's UGizmoCircleComponent class.
// Due to the needs of 4D, we have to implement our own class (we need 2 colors).
UCLASS(MinimalAPI)
class UEditorTransformGizmoBiplaneMesh4D : public UEditorTransformGizmoBaseMesh4D
{
	GENERATED_BODY()

private:
	FLinearColor SecondaryNormalColor = FLinearColor::Red;
	FLinearColor SecondaryHighlightColor = FLinearColor::Red;
	FVector SecondaryTargetPosition = FVector();

public:
	void SetSecondaryColors(const FLinearColor& InSecondaryNormalColor, const FLinearColor& InSecondaryHighlightColor);
	FLinearColor GetCurrentSecondaryColor() const { return bIsHighlighted ? SecondaryHighlightColor : SecondaryNormalColor; }

	void SetSecondaryTargetPosition(const FVector& InSecondaryPosition);
	FVector GetSecondaryTargetPosition() const { return SecondaryTargetPosition; }

	virtual FBoxSphereBounds CalcBounds(const FTransform& InLocalToWorld) const override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UEditorTransformGizmoBiplaneMesh4D() : Super() {}
};
