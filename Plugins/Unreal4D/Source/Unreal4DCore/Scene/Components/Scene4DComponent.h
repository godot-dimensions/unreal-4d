#pragma once

#include "../../Math/Rect4D.h"
#include "../../Math/Transform4D.h"
#include "Components/PrimitiveComponent.h"

#include "Scene4DComponent.generated.h"

UENUM(BlueprintType, Category = "Unreal 4D|Component")
enum class ERotationEditMode4D : uint8
{
	Euler4D,
	Euler4D_Uniform_Scale,
	Basis4D,
};

#if WITH_EDITOR
class FScene4DComponentDetails;
#endif // WITH_EDITOR

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAnyTransform4DChanged, UScene4DComponent*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransform4DChanged, UScene4DComponent*, InChanged4DComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisibilityChanged4D, bool, bIsVisible);

// Class which provides a 4D transform that exists in the scene hierarchy.
// Components with a physical or visible surface should extend UPrimitive4DComponent instead.
UCLASS(BlueprintType, Category = "Unreal 4D|Component", meta = (BlueprintSpawnableComponent))
class UNREAL4DCORE_API UScene4DComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	friend class FScene4DComponentDetails;
	friend class FUnreal4DEditorModule;

private:
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Component|Transform", meta=(AllowPrivateAccess="true", DisplayPriority=1))
	FVector4 _localPosition4D = FVector4::Zero();
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Component|Transform", meta=(AllowPrivateAccess="true", DisplayPriority=2))
	ERotationEditMode4D _rotationEditMode = ERotationEditMode4D::Euler4D;
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Component|Transform", meta=(AllowPrivateAccess="true", DisplayPriority=3))
	FMatrix _localBasis4D = FMatrix::Identity;
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Component|Transform", meta=(AllowPrivateAccess="true", DisplayPriority=4))
	FEuler4D _localRotationDegrees4D = FEuler4D::Zero();
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Component|Transform", meta=(AllowPrivateAccess="true", DisplayPriority=5))
	FVector4 _localScale4D = FVector4(1.0, 1.0, 1.0, 1.0);
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Component|Transform", meta=(AllowPrivateAccess="true", DisplayPriority=6))
	double _localUniformScale4D = 1.0;
#if WITH_EDITOR
	// There aren't any infinite loops without this, but ChatGPT insisted
	// this is an important safeguard, so just in case, it's here.
	bool _bSyncingFromEditor = false;
#endif // WITH_EDITOR

	void PropagateVisibilityChanged4D(bool bNewVisibility);

public:
	void SyncTransform4DFromArchetype(); // Public for editor purposes, but doesn't need to be called externally otherwise.
	void UpdatedTransform4D(); // Public for editor purposes, but doesn't need to be called externally otherwise.

	UPROPERTY(Transient, BlueprintAssignable, DuplicateTransient, NonTransactional, SkipSerialization, Category="Unreal 4D|Component|Transform")
	FOnTransform4DChanged OnTransform4DChangedDelegate;

	UPROPERTY(Transient, BlueprintAssignable, DuplicateTransient, NonTransactional, SkipSerialization, Category="Unreal 4D|Component|Visibility")
	FOnVisibilityChanged4D OnVisibilityChanged4D;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component")
	UScene4DComponent* GetParent4DComponent() const;

	// Finds the parent 4D component of a scene component by walking the attachment hierarchy.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component")
	static UScene4DComponent* FindParent4DComponent(USceneComponent* InComponent);

	// Transform altering methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void ApplyScale(const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void TranslateLocal(const FVector4& InTranslation);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform", meta = (CPP_Default_InGlobalUp = "(X=0.0,Y=0.0,Z=1.0,W=0.0)"))
	void LookAt(const FVector4& InGlobalTarget, const FVector4& InGlobalUp = FVector4(0.0, 0.0, 1.0, 0.0));

	// Rotation altering methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void RotateRadians(const FEuler4D& InEulerRadians);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void RotateLocalRadians(const FEuler4D& InLocalEulerRadians);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void RotateDegrees(const FEuler4D& InEulerDegrees);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void RotateLocalDegrees(const FEuler4D& InLocalEulerDegrees);

	// Local transform and basis.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FTransform4D GetLocalTransform4D() const { return FTransform4D(_localBasis4D, _localPosition4D); }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalTransform4D(const FTransform4D& InTransform);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FMatrix GetLocalBasis4D() const { return _localBasis4D; }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalBasis4D(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalTransform4Dfrom3D(const FTransform& InTransform3D);

	// Local transform components.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FVector4 GetLocalPosition4D() const { return _localPosition4D; }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalPosition4D(const FVector4& InPosition);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FEuler4D GetLocalRotationRadians4D() { return _localRotationDegrees4D.DegToRad(); }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalRotationRadians4D(const FEuler4D& InRotationEulerRadians);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FEuler4D GetLocalRotationDegrees4D() { return _localRotationDegrees4D; }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalRotationDegrees4D(const FEuler4D& InRotationEulerDegrees);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FVector4 GetLocalScale4D() { return _localScale4D; }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalScale4D(const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	double GetLocalUniformScale4D() const { return _localUniformScale4D; }
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetLocalUniformScale4D(const double InUniformScale);

	// Global transform and basis.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FTransform4D GetGlobalTransform4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FMatrix GetGlobalBasis4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalTransform4D(const FTransform4D& InTransform);
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalBasis4D(const FMatrix& InBasis);

	// Global transform components.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FVector4 GetGlobalPosition4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalPosition4D(const FVector4& InPosition);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FEuler4D GetGlobalRotationRadians4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalRotationRadians4D(const FEuler4D& InRotationEulerRadians);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FEuler4D GetGlobalRotationDegrees4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalRotationDegrees4D(const FEuler4D& InRotationEulerDegrees);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	FVector4 GetGlobalScale4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalScale4D(const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Transform")
	double GetGlobalUniformScale4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Transform")
	void SetGlobalUniformScale4D(const double InUniformScale);

	// Visibility.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Component|Visibility")
	bool IsVisibleInTree4D() const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Component|Visibility")
	void SetVisible4D(bool bNewVisibility);

	// Rect bounds.
	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InToTarget) const;
	FRect4D GetRectBoundsGlobal4D(const FTransform4D& InToTarget) const;
	FRect4D GetRectBoundsGlobalRecursive4D(const FTransform4D& InToTarget) const;

	UScene4DComponent(const FObjectInitializer& Init);

protected:
	void NotifyTransform4DChanged();
	virtual void OnTransform4DChanged() {}
	virtual void OnAttachmentHierarchyChanged4D();

	void OnRegister() override;
	void OnAttachmentChanged() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	void PostEditHandlePropertyName(const FName& PropertyName);
#endif // WITH_EDITOR
};
