#include "Transform4D.h"

// Misc methods.

double FTransform4D::Determinant() const
{
	return Basis.Determinant();
}

FTransform4D FTransform4D::Lerp(const FTransform4D& InTransform, double InWeight) const
{
	FTransform4D interp;
	interp.Basis = UBasis4D::Basis4D_Lerp(Basis, InTransform.Basis, InWeight);
	interp.Origin = FMath::Lerp(Origin, InTransform.Origin, InWeight);
	return interp;
}

bool FTransform4D::IsEqualApprox(const FTransform4D& InTransform) const
{
	return Basis.Equals(InTransform.Basis, UNREAL4D_COMPARISON_EPSILON) && Origin.Equals(InTransform.Origin, UNREAL4D_COMPARISON_EPSILON);
}

void FTransform4D::TranslateLocal(const FVector4& InTranslation)
{
	Origin += Basis.TransformFVector4(InTranslation);
}

FTransform4D FTransform4D::TranslatedLocal(const FVector4& InTranslation) const
{
	FTransform4D t = *this;
	t.TranslateLocal(InTranslation);
	return t;
}

// Transformation methods.

FTransform4D FTransform4D::Compose(const FTransform4D& InChildTransform) const
{
	// This uses standard math order: parent * child, NOT Unreal's FTransform order of child * parent.
	return *this * InChildTransform;
}

// Returns a transform that, when applied to this transform from the left
// (`ret.Compose(this)`), transforms this into the `InTo` transform.
FTransform4D FTransform4D::TransformTo(const FTransform4D& InTo) const
{
	return InTo * Inverse();
}

FVector4 FTransform4D::TransformPosition(const FVector4& InVector) const
{
	return Basis.TransformFVector4(InVector) + Origin;
}

TArray<FVector4> FTransform4D::TransformPositionMany(const TArray<FVector4>& InVectors) const
{
	TArray<FVector4> array;
	array.SetNum(InVectors.Num());

	for (int32 i = 0; i < InVectors.Num(); ++i)
	{
		array[i] = TransformPosition(InVectors[i]);
	}
	return array;
}

FVector4 FTransform4D::InverseTransformPosition(const FVector4& InVector) const
{
	return Inverse().TransformPosition(InVector);
}

TArray<FVector4> FTransform4D::InverseTransformPositionMany(const TArray<FVector4>& InVectors) const
{
	TArray<FVector4> array;
	array.SetNum(InVectors.Num());

	for (int32 i = 0; i < InVectors.Num(); ++i)
	{
		array[i] = InverseTransformPosition(InVectors[i]);
	}
	return array;
}

FVector4 FTransform4D::TransposeTransformPosition(const FVector4& InVector) const
{
	return UBasis4D::Basis4D_TransposeTransform(Basis, InVector - Origin);
}

TArray<FVector4> FTransform4D::TransposeTransformPositionMany(const TArray<FVector4>& InVectors) const
{
	TArray<FVector4> array;
	array.SetNum(InVectors.Num());

	for (int32 i = 0; i < InVectors.Num(); ++i)
	{
		array[i] = TransposeTransformPosition(InVectors[i]);
	}
	return array;
}

// Inversion methods.

FTransform4D FTransform4D::Inverse() const
{
	FTransform4D ret = FTransform4D(UBasis4D::Basis4D_Inverse(Basis));
	ret.Origin = ret.Basis.TransformFVector4(-Origin);
	return ret;
}

FTransform4D FTransform4D::InverseBasis() const
{
	return FTransform4D(UBasis4D::Basis4D_Inverse(Basis));
}

FTransform4D FTransform4D::InverseTransposed() const
{
	FTransform4D ret = FTransform4D(Basis.GetTransposed());
	ret.Origin = ret.Basis.TransformFVector4(-Origin);
	return ret;
}

// Rotation methods.

void FTransform4D::RotateGlobal(const FEuler4D& InEuler)
{
	FMatrix rotation = InEuler.RadiansToBasis();
	Basis = UBasis4D::Basis4D_Compose(rotation, Basis);
	Origin = rotation.TransformFVector4(Origin);
}

FTransform4D FTransform4D::RotatedGlobal(const FEuler4D& InEuler) const
{
	FMatrix rotation = InEuler.RadiansToBasis();
	return FTransform4D(UBasis4D::Basis4D_Compose(rotation, Basis), rotation.TransformFVector4(Origin));
}

void FTransform4D::RotateLocal(const FEuler4D& InEuler)
{
	Basis = UBasis4D::Basis4D_Compose(Basis, InEuler.RadiansToBasis());
}

FTransform4D FTransform4D::RotatedLocal(const FEuler4D& InEuler) const
{
	return FTransform4D(UBasis4D::Basis4D_Compose(Basis, InEuler.RadiansToBasis()), Origin);
}

FEuler4D FTransform4D::GetRotation() const
{
	return FEuler4D::RadiansFromBasis(Basis);
}

void FTransform4D::SetRotation(const FEuler4D& InEuler)
{
	Basis = InEuler.RadiansToBasis();
}

FEuler4D FTransform4D::GetRotationDegrees() const
{
	return FEuler4D::RadiansFromBasis(Basis).RadToDeg();
}

void FTransform4D::SetRotationDegrees(const FEuler4D& InEuler)
{
	Basis = InEuler.DegToRad().RadiansToBasis();
}

// Scale methods.

void FTransform4D::ScaleGlobal(const FVector4& InScale)
{
	Basis = UBasis4D::Basis4D_ScaledGlobal(Basis, InScale);
	Origin *= InScale;
}

FTransform4D FTransform4D::ScaledGlobal(const FVector4& InScale) const
{
	FTransform4D t = *this;
	t.ScaleGlobal(InScale);
	return t;
}

void FTransform4D::ScaleLocal(const FVector4& InScale)
{
	Basis = UBasis4D::Basis4D_ScaledLocal(Basis, InScale);
}

FTransform4D FTransform4D::ScaledLocal(const FVector4& InScale) const
{
	FTransform4D t = *this;
	t.ScaleLocal(InScale);
	return t;
}

void FTransform4D::ScaleUniformGlobal(const double InUniformScale)
{
	Basis = UBasis4D::Basis4D_ScaledUniform(Basis, InUniformScale);
	Origin *= InUniformScale;
}

FTransform4D FTransform4D::ScaledUniformGlobal(const double InUniformScale) const
{
	FTransform4D t = *this;
	t.ScaleUniformGlobal(InUniformScale);
	return t;
}

void FTransform4D::ScaleUniformLocal(const double InUniformScale)
{
	Basis = UBasis4D::Basis4D_ScaledUniform(Basis, InUniformScale);
}

FTransform4D FTransform4D::ScaledUniformLocal(const double InUniformScale) const
{
	FTransform4D t = *this;
	t.ScaleUniformLocal(InUniformScale);
	return t;
}

FVector4 FTransform4D::GetScale() const
{
	return UBasis4D::Basis4D_GetScale(Basis);
}

FVector4 FTransform4D::GetScaleAbs() const
{
	return UBasis4D::Basis4D_GetScaleAbs(Basis);
}

void FTransform4D::SetScale(const FVector4& InScale)
{
	Basis = UBasis4D::Basis4D_WithScale(Basis, InScale);
}

double FTransform4D::GetUniformScale() const
{
	return UBasis4D::Basis4D_GetUniformScale(Basis);
}

void FTransform4D::SetUniformScale(const double InUniformScale)
{
	Basis = UBasis4D::Basis4D_WithUniformScale(Basis, InUniformScale);
}

// Validation methods.

void FTransform4D::Conformalize()
{
	Basis = UBasis4D::Basis4D_Conformalized(Basis);
}

FTransform4D FTransform4D::Conformalized() const
{
	return FTransform4D(UBasis4D::Basis4D_Conformalized(Basis), Origin);
}

void FTransform4D::Normalize()
{
	Basis = UBasis4D::Basis4D_Normalized(Basis);
}

FTransform4D FTransform4D::Normalized() const
{
	return FTransform4D(UBasis4D::Basis4D_Normalized(Basis), Origin);
}

void FTransform4D::Orthonormalize()
{
	Basis = UBasis4D::Basis4D_Orthonormalized(Basis);
}

FTransform4D FTransform4D::Orthonormalized() const
{
	return FTransform4D(UBasis4D::Basis4D_Orthonormalized(Basis), Origin);
}

void FTransform4D::Orthogonalize()
{
	Basis = UBasis4D::Basis4D_Orthogonalized(Basis);
}

FTransform4D FTransform4D::Orthogonalized() const
{
	return FTransform4D(UBasis4D::Basis4D_Orthogonalized(Basis), Origin);
}

// Operators.

bool FTransform4D::operator==(const FTransform4D& InTransform) const
{
	return (Basis == InTransform.Basis && Origin == InTransform.Origin);
}

bool FTransform4D::operator!=(const FTransform4D& InTransform) const
{
	return (Basis != InTransform.Basis || Origin != InTransform.Origin);
}

void FTransform4D::operator+=(const FTransform4D& InTransform)
{
	Origin += InTransform.Origin;
	Basis += InTransform.Basis;
}

FTransform4D FTransform4D::operator+(const FTransform4D& InTransform) const
{
	return FTransform4D(Basis + InTransform.Basis, Origin + InTransform.Origin);
}

void FTransform4D::operator*=(const FTransform4D& InTransform)
{
	Origin = TransformPosition(InTransform.Origin);
	Basis = UBasis4D::Basis4D_Compose(Basis, InTransform.Basis);
}

FTransform4D FTransform4D::operator*(const FTransform4D& InTransform) const
{
	// This uses standard math order: parent * child, NOT Unreal's FTransform order of child * parent.
	return FTransform4D(
			UBasis4D::Basis4D_Compose(Basis, InTransform.Basis),
			TransformPosition(InTransform.Origin));
}

FMatrix FTransform4D::operator*(const FMatrix& InBasis) const
{
	return UBasis4D::Basis4D_Compose(Basis, InBasis);
}

FVector4 FTransform4D::operator*(const FVector4& InVector) const
{
	return Basis.TransformFVector4(InVector) + Origin;
}

void FTransform4D::operator*=(const double InVal)
{
	Origin *= InVal;
	Basis = UBasis4D::Basis4D_ScaledUniform(Basis, InVal);
}

FTransform4D FTransform4D::operator*(const double InVal) const
{
	return FTransform4D(UBasis4D::Basis4D_ScaledUniform(Basis, InVal), Origin * InVal);
}

void FTransform4D::operator/=(const double InVal)
{
	Origin /= InVal;
	Basis = UBasis4D::Basis4D_ScaledUniform(Basis, 1.0 / InVal);
}

FTransform4D FTransform4D::operator/(const double InVal) const
{
	return FTransform4D(UBasis4D::Basis4D_ScaledUniform(Basis, 1.0 / InVal), Origin / InVal);
}

FTransform4D::operator FString() const
{
	return FString::Printf(TEXT("[X: (%f, %f, %f, %f), Y: (%f, %f, %f, %f), Z: (%f, %f, %f, %f), W: (%f, %f, %f, %f), O: (%f, %f, %f, %f)]"),
		Basis.M[0][0], Basis.M[0][1], Basis.M[0][2], Basis.M[0][3],
		Basis.M[1][0], Basis.M[1][1], Basis.M[1][2], Basis.M[1][3],
		Basis.M[2][0], Basis.M[2][1], Basis.M[2][2], Basis.M[2][3],
		Basis.M[3][0], Basis.M[3][1], Basis.M[3][2], Basis.M[3][3],
		Origin.X, Origin.Y, Origin.Z, Origin.W);
}

// Conversion.

FTransform4D FTransform4D::From3D(const FTransform& InFrom3D)
{
	const FQuat Quat = InFrom3D.GetRotation();
	const FVector Location = InFrom3D.GetLocation();
	const FMatrix basis4D = UBasis4D::Basis4D_From3D(InFrom3D.ToMatrixWithScale());
	return FTransform4D(basis4D, FVector4(Location.X, Location.Y, Location.Z, 0.0));
}

FTransform FTransform4D::To3D(const bool bOrthonormalized) const
{
	FMatrix basis3D = UBasis4D::Basis4D_To3D(Basis, bOrthonormalized);
	const FVector scale3D = basis3D.ExtractScaling();
	const FQuat rotation3D = FQuat(basis3D);
	const FVector translation3D = FVector(Origin.X, Origin.Y, Origin.Z);
	return FTransform(rotation3D, translation3D, scale3D);
}

FTransform4D FTransform4D::FromArray(const TArray<double>& InArray)
{
	check(InArray.Num() >= 20);
	return FTransform4D(
		UBasis4D::Basis4D_FromNumbers(InArray[0], InArray[1], InArray[2], InArray[3],
				InArray[4], InArray[5], InArray[6], InArray[7],
				InArray[8], InArray[9], InArray[10], InArray[11],
				InArray[12], InArray[13], InArray[14], InArray[15]),
		FVector4(InArray[16], InArray[17], InArray[18], InArray[19]));
}

TArray<double> FTransform4D::ToArray() const
{
	TArray<double> arr;
	arr.SetNum(20);
	arr[0] = Basis.M[0][0];
	arr[1] = Basis.M[0][1];
	arr[2] = Basis.M[0][2];
	arr[3] = Basis.M[0][3];
	arr[4] = Basis.M[1][0];
	arr[5] = Basis.M[1][1];
	arr[6] = Basis.M[1][2];
	arr[7] = Basis.M[1][3];
	arr[8] = Basis.M[2][0];
	arr[9] = Basis.M[2][1];
	arr[10] = Basis.M[2][2];
	arr[11] = Basis.M[2][3];
	arr[12] = Basis.M[3][0];
	arr[13] = Basis.M[3][1];
	arr[14] = Basis.M[3][2];
	arr[15] = Basis.M[3][3];
	arr[16] = Origin.X;
	arr[17] = Origin.Y;
	arr[18] = Origin.Z;
	arr[19] = Origin.W;
	return arr;
}

FString FTransform4D::ToString() const
{
	return this->operator FString();
}

static_assert(sizeof(FTransform4D) == 20 * sizeof(double), "FTransform4D must be 20 doubles or 160 bytes in size");
