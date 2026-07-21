#pragma once

#include "PhysicsBody4D.h"

#include "../../Math/Bivector4D.h"

#include "DynamicBody4D.generated.h"

// Body component class that moves in a physically simulated way controlled by the physics engine.
// This is used for things like props, vehicles, and other objects that should be affected by forces.
UCLASS(Category = "Unreal 4D|Physics|Body")
class UNREAL4DCORE_API UDynamicBody4DComponent : public UPhysicsBody4DComponent
{
	GENERATED_BODY()

	FBivector4D TransformBivector(const FMatrix& InBasis, const FBivector4D& InBivector);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	FBivector4D AngularVelocityRadians = FBivector4D(); // Radians per second.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body", meta = (ClampMin = "0.000001"))
	double Mass = 1.0; // Kilograms. Must be greater than 0.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	double GravityScale = 1.0; // Unitless scale factor for gravity.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	FVector4 LinearVelocity = FVector4(0.0, 0.0, 0.0, 0.0); // Centimeters per second.

	void ApplyAcceleration(const FVector4& InAcceleration, const double InDeltaTime = -1.0);
	void ApplyLocalAcceleration(const FVector4& InLocalAcceleration, const double InDeltaTime = -1.0);

	void ApplyForce(const FVector4& InForce, const FVector4& InPositionOffset = FVector4(0.0, 0.0, 0.0, 0.0), const double InDeltaTime = -1.0);
	void ApplyImpulse(const FVector4& InImpulse, const FVector4& InPositionOffset = FVector4(0.0, 0.0, 0.0, 0.0));
	void ApplyLocalForce(const FVector4& InLocalForce, const FVector4& InLocalPositionOffset = FVector4(0.0, 0.0, 0.0, 0.0), const double InDeltaTime = -1.0);
	void ApplyLocalImpulse(const FVector4& InLocalImpulse, const FVector4& InLocalPositionOffset = FVector4(0.0, 0.0, 0.0, 0.0));

	void ApplyTorque(const FBivector4D& InTorque, const double InDeltaTime = -1.0);
	void ApplyTorqueImpulse(const FBivector4D& InTorqueImpulse);
	void ApplyLocalTorque(const FBivector4D& InTorque, const double InDeltaTime = -1.0);
	void ApplyLocalTorqueImpulse(const FBivector4D& InTorqueImpulse);

	FVector4 GetScaledGravity() const;

	FBivector4D GetAngularVelocityDegrees() const;
	void SetAngularVelocityDegrees(const FBivector4D& InAngularVelocityDegrees);
};
