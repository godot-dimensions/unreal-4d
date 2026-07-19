#pragma once

#include "Euler4D.h"

#include "Transform4D.generated.h"

USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Transform")
struct UNREAL4DCORE_API FTransform4D
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Transform")
	FMatrix Basis = FMatrix::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Transform")
	FVector4 Origin = FVector4::Zero();

	// Misc methods.
	double Determinant() const;
	FTransform4D Lerp(const FTransform4D& InTransform, double InWeight) const;
	bool IsEqualApprox(const FTransform4D& InTransform) const;
	void TranslateLocal(const FVector4& InTranslation);
	FTransform4D TranslatedLocal(const FVector4& InTranslation) const;
	static FTransform4D Identity() { return FTransform4D(FMatrix::Identity, FVector4::Zero()); }

	// Transformation methods.
	FTransform4D Compose(const FTransform4D& InChildTransform) const;
	FTransform4D TransformTo(const FTransform4D& InTo) const;
	FVector4 TransformPosition(const FVector4& InVector) const;
	TArray<FVector4> TransformPositionMany(const TArray<FVector4>& InVectors) const;
	FVector4 InverseTransformPosition(const FVector4& InVector) const;
	TArray<FVector4> InverseTransformPositionMany(const TArray<FVector4>& InVectors) const;
	FVector4 TransposeTransformPosition(const FVector4& InVector) const;
	TArray<FVector4> TransposeTransformPositionMany(const TArray<FVector4>& InVectors) const;

	// Inversion methods.
	FTransform4D Inverse() const;
	FTransform4D InverseBasis() const;
	FTransform4D InverseTransposed() const;

	// Rotation methods.
	void RotateGlobal(const FEuler4D& InEuler);
	FTransform4D RotatedGlobal(const FEuler4D& InEuler) const;
	void RotateLocal(const FEuler4D& InEuler);
	FTransform4D RotatedLocal(const FEuler4D& InEuler) const;
	FEuler4D GetRotation() const;
	void SetRotation(const FEuler4D& InEuler);
	FEuler4D GetRotationDegrees() const;
	void SetRotationDegrees(const FEuler4D& InEuler);

	// Scale methods.
	void ScaleGlobal(const FVector4& InScale);
	FTransform4D ScaledGlobal(const FVector4& InScale) const;
	void ScaleLocal(const FVector4& InScale);
	FTransform4D ScaledLocal(const FVector4& InScale) const;
	void ScaleUniformGlobal(double InUniformScale);
	FTransform4D ScaledUniformGlobal(double InUniformScale) const;
	void ScaleUniformLocal(double InUniformScale);
	FTransform4D ScaledUniformLocal(double InUniformScale) const;
	FVector4 GetScale() const;
	FVector4 GetScaleAbs() const;
	void SetScale(const FVector4& InScale);
	double GetUniformScale() const;
	void SetUniformScale(double InUniformScale);

	// Validation methods.
	void Conformalize();
	FTransform4D Conformalized() const;
	void Normalize();
	FTransform4D Normalized() const;
	void Orthonormalize();
	FTransform4D Orthonormalized() const;
	void Orthogonalize();
	FTransform4D Orthogonalized() const;

	// Helper setters/getters.
	FMatrix GetBasis() const { return Basis; }
	void SetBasis(const FMatrix& InBasis) { Basis = InBasis; }
	FVector4 GetOrigin() const { return Origin; }
	void SetOrigin(const FVector4& InOrigin) { Origin = InOrigin; }

	// Operators.
	FVector4 operator[](int32 InAxis) const {
		check((unsigned int)InAxis < 5);
		if (InAxis == 4)
		{
			return Origin;
		}
		return UBasis4D::Basis4D_GetRow(Basis, InAxis);
	}

	bool operator==(const FTransform4D& InTransform) const;
	bool operator!=(const FTransform4D& InTransform) const;
	void operator+=(const FTransform4D& InTransform);
	FTransform4D operator+(const FTransform4D& InTransform) const;
	void operator*=(const FTransform4D& InTransform);
	FTransform4D operator*(const FTransform4D& InTransform) const;
	FMatrix operator*(const FMatrix& InBasis) const;
	FVector4 operator*(const FVector4& InVector) const;
	void operator*=(double InVal);
	FTransform4D operator*(double InVal) const;
	void operator/=(double InVal);
	FTransform4D operator/(double InVal) const;
	operator FString() const;

	// Conversion.
	static FTransform4D From3D(const FTransform& InFrom3D);
	FTransform To3D(bool bOrthonormalized = false) const;
	static FTransform4D FromArray(const TArray<double>& InArray);
	TArray<double> ToArray() const;
	FString ToString() const;

	// Constructors.
	FTransform4D() = default;

	FTransform4D(const FMatrix& InBasis, const FVector4& InOrigin = FVector4(0.0, 0.0, 0.0, 0.0))
		: Basis(InBasis), Origin(InOrigin)
	{
	}

	FTransform4D(const FVector4& InX, const FVector4& InY, const FVector4& InZ, const FVector4& InW, const FVector4& InOrigin = FVector4(0.0, 0.0, 0.0, 0.0))
		: Basis(UBasis4D::Basis4D_FromVectors(InX, InY, InZ, InW)), Origin(InOrigin)
	{
	}

	FTransform4D(double InXX, double InXY, double InXZ, double InXW,
	               double InYX, double InYY, double InYZ, double InYW,
	               double InZX, double InZY, double InZZ, double InZW,
	               double InWX, double InWY, double InWZ, double InWW,
	               double InOX = 0.0, double InOY = 0.0, double InOZ = 0.0, double InOW = 0.0)
		: Basis(UBasis4D::Basis4D_FromNumbers(InXX, InXY, InXZ, InXW, InYX, InYY, InYZ, InYW, InZX, InZY, InZZ, InZW, InWX, InWY, InWZ, InWW))
		, Origin(FVector4(InOX, InOY, InOZ, InOW))
	{
	}
};
