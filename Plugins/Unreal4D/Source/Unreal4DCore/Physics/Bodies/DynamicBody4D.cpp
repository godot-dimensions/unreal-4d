#include "DynamicBody4D.h"

FBivector4D UDynamicBody4DComponent::TransformBivector(const FMatrix& InBasis, const FBivector4D& InBivector)
{
	const FVector4 x = InBasis.TransformFVector4(FVector4(1.0, 0.0, 0.0, 0.0));
	const FVector4 y = InBasis.TransformFVector4(FVector4(0.0, 1.0, 0.0, 0.0));
	const FVector4 z = InBasis.TransformFVector4(FVector4(0.0, 0.0, 1.0, 0.0));
	const FVector4 w = InBasis.TransformFVector4(FVector4(0.0, 0.0, 0.0, 1.0));

	return FBivector4D(x, y) * InBivector.XY
			+ FBivector4D(x, z) * InBivector.XZ
			+ FBivector4D(x, w) * InBivector.XW
			+ FBivector4D(y, z) * InBivector.YZ
			+ FBivector4D(y, w) * InBivector.YW
			+ FBivector4D(z, w) * InBivector.ZW;
}

void UDynamicBody4DComponent::ApplyAcceleration(const FVector4& InAcceleration, const double InDeltaTime)
{
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	// Acceleration is in cm/s^2, so multiplying that by s gives cm/s for velocity.
	const FVector4 velocityChange = InAcceleration * deltaTime;
	LinearVelocity += velocityChange;
}

void UDynamicBody4DComponent::ApplyLocalAcceleration(const FVector4& InLocalAcceleration, const double InDeltaTime)
{
	const FMatrix globalBasis = GetGlobalBasis4D();
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	// Acceleration is in cm/s^2, so multiplying that by s gives cm/s for velocity.
	const FVector4 velocityChange = globalBasis.TransformFVector4(InLocalAcceleration * deltaTime);
	LinearVelocity += velocityChange;
}

void UDynamicBody4DComponent::ApplyForce(const FVector4& InForce, const FVector4& InPositionOffset, const double InDeltaTime)
{
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	// Force is in kg*cm/s^2, so multiplying that by s/kg gives cm/s for velocity.
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	const FVector4 velocityChange = InForce * (deltaTime / safeMass);
	LinearVelocity += velocityChange;
	// A force at a position may also cause a torque.
	if (InPositionOffset != FVector4(0.0, 0.0, 0.0, 0.0))
	{
		const FBivector4D effectiveTorque = FBivector4D(InPositionOffset, velocityChange);
		AngularVelocityRadians = AngularVelocityRadians + effectiveTorque;
	}
}

void UDynamicBody4DComponent::ApplyImpulse(const FVector4& InImpulse, const FVector4& InPositionOffset)
{
	// Impulse is in kg*cm/s, so dividing that by kg gives cm/s for velocity.
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	const FVector4 velocityChange = InImpulse / safeMass;
	LinearVelocity += velocityChange;
	// An impulse at a position may also cause a torque.
	if (InPositionOffset != FVector4(0.0, 0.0, 0.0, 0.0))
	{
		const FBivector4D effectiveTorque = FBivector4D(InPositionOffset, velocityChange);
		AngularVelocityRadians = AngularVelocityRadians + effectiveTorque;
	}
}

void UDynamicBody4DComponent::ApplyLocalForce(const FVector4& InLocalForce, const FVector4& InLocalPositionOffset, const double InDeltaTime)
{
	const FMatrix globalBasis = GetGlobalBasis4D();
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	// Force is in kg*cm/s^2, so multiplying that by s/kg gives cm/s for velocity.
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	const FVector4 velocityChange = globalBasis.TransformFVector4(InLocalForce * (deltaTime / safeMass));
	LinearVelocity += velocityChange;
	// A force at a position may also cause a torque.
	if (InLocalPositionOffset != FVector4(0.0, 0.0, 0.0, 0.0))
	{
		const FVector4 globalPositionOffset = globalBasis.TransformFVector4(InLocalPositionOffset);
		const FBivector4D effectiveGlobalTorque = FBivector4D(globalPositionOffset, velocityChange);
		AngularVelocityRadians = AngularVelocityRadians + effectiveGlobalTorque;
	}
}

void UDynamicBody4DComponent::ApplyLocalImpulse(const FVector4& InLocalImpulse, const FVector4& InLocalPositionOffset)
{
	const FMatrix globalBasis = GetGlobalBasis4D();
	// Impulse is in kg*cm/s, so dividing that by kg gives cm/s for velocity.
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	const FVector4 velocityChange = globalBasis.TransformFVector4(InLocalImpulse / safeMass);
	LinearVelocity += velocityChange;
	// An impulse at a position may also cause a torque.
	if (InLocalPositionOffset != FVector4(0.0, 0.0, 0.0, 0.0))
	{
		const FVector4 globalPositionOffset = globalBasis.TransformFVector4(InLocalPositionOffset);
		const FBivector4D effectiveGlobalTorque = FBivector4D(globalPositionOffset, velocityChange);
		AngularVelocityRadians = AngularVelocityRadians + effectiveGlobalTorque;
	}
}

void UDynamicBody4DComponent::ApplyTorque(const FBivector4D& InTorque, const double InDeltaTime)
{
	// TODO: None of the code in this class considers the inertia tensor.
	// Non-trivial physical objects can be rotated more easily in different directions.
	// However, for now, this works pretty well, and works perfectly for simple objects (boxes, spheres, etc).
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	AngularVelocityRadians = AngularVelocityRadians + InTorque * (deltaTime / safeMass);
}

void UDynamicBody4DComponent::ApplyTorqueImpulse(const FBivector4D& InTorqueImpulse)
{
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	AngularVelocityRadians = AngularVelocityRadians + InTorqueImpulse / safeMass;
}

void UDynamicBody4DComponent::ApplyLocalTorque(const FBivector4D& InTorque, const double InDeltaTime)
{
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	AngularVelocityRadians = AngularVelocityRadians + TransformBivector(GetGlobalBasis4D(), InTorque * deltaTime) / safeMass;
}

void UDynamicBody4DComponent::ApplyLocalTorqueImpulse(const FBivector4D& InTorqueImpulse)
{
	const double safeMass = FMath::Max(Mass, UNREAL4D_COMPARISON_EPSILON);
	AngularVelocityRadians = AngularVelocityRadians + TransformBivector(GetGlobalBasis4D(), InTorqueImpulse) / safeMass;
}

FVector4 UDynamicBody4DComponent::GetScaledGravity() const
{
	return GetGravity() * GravityScale;
}

FBivector4D UDynamicBody4DComponent::GetAngularVelocityDegrees() const
{
	return AngularVelocityRadians * (360.0 / UNREAL4D_TAU);
}

void UDynamicBody4DComponent::SetAngularVelocityDegrees(const FBivector4D& InAngularVelocityDegrees)
{
	AngularVelocityRadians = InAngularVelocityDegrees * (UNREAL4D_TAU / 360.0);
}
