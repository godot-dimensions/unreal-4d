#if WITH_DEV_AUTOMATION_TESTS

// The bivector and QuatPair tests require a lot of the same setup, so they're both here together.

#include "Misc/AutomationTest.h"

#include "Math/UnrealMath.h"
#include "../../../../Unreal4DCore/Math/Bivector4D.h"
#include "../../../../Unreal4DCore/Math/Euler4D.h"
#include "../../../../Unreal4DCore/Math/QuaternionPair.h"
#include <random>
#include <vector>

using namespace UE::Math; // TODO: check if this is still doing anything.

constexpr double UNREAL4D_ROTATION_TEST_EPSILON = 2e-14; // If the functions tested here have errors as large as UE_SMALL_NUMBER, something has gone wrong. This is about 100 ULPs for doubles between 1 and 2.
const double UNREAL4D_ROTATION_TEST_BIG_EPSILON = FGenericPlatformMath::Sqrt(UNREAL4D_ROTATION_TEST_EPSILON); // except some of the tests involving things like division may have larger errors, but still, we can be precise about how large.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_RotationTests, "Unreal4D.RotationTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

class FTestCase {
public:
	FVector4 VA;
	FVector4 VB;
	FVector4 VC;
	FVector4 VD;
	FVector4 VE;
	FVector4 VF;
	FVector4 VPB; // Perpendicular to VA
	FVector4 VPC; // Perpendicular to VA and VPB
	FVector4 VPD; // Perpendicular to VA, VPB and VPC
	FQuaternionPair QA;
	FQuaternionPair QB;
	FQuaternionPair QC;
	double S;

	FTestCase(std::vector<double> Data) :
		VA(ExtractVector(0, Data)),
		VB(ExtractVector(1, Data)),
		VC(ExtractVector(2, Data)),
		VD(ExtractVector(3, Data)),
		VE(ExtractVector(4, Data)),
		VF(ExtractVector(5, Data)),
		VPB(ExtractVector(6, Data)),
		VPC(ExtractVector(7, Data)),
		VPD(ExtractVector(8, Data)),
		QA(FQuaternionPair(ExtractQuat(9,Data), ExtractQuat(10, Data))),
		QB(FQuaternionPair(ExtractQuat(11,Data), ExtractQuat(12, Data))),
		QC(FQuaternionPair(ExtractQuat(13,Data), ExtractQuat(14, Data))),
		S(Data[4*15]) {
		Orthogonalize(VPB, VA);
		Orthogonalize(VPC, VA);
		Orthogonalize(VPC, VPB);
		Orthogonalize(VPD, VA);
		Orthogonalize(VPD, VPB);
		Orthogonalize(VPD, VPC);
	}

	// Some of the identities may fail to hold for very small vectors, because of numerical precision issues. This is inevitable because a lot of the functions are discontinuous or ill-defined at 0.
	bool IsSuitable(double MinSize) const {
		return
			VA.Size() > MinSize
			// VB through VF are allowed to be small.
			&& VPB.Size() > MinSize
			&& VPC.Size() > MinSize
			&& VPD.Size() > MinSize
			&& QA.ScaleFactor() > 0.9 // These are normalized, so they're either tiny or near standard size (1).
			&& QB.ScaleFactor() > 0.9
			&& QC.ScaleFactor() > 0.9
			&& FMath::Abs(S) > MinSize;
	}

	static const uint32 REQUIRED_DATA = 4*15+1;

	private:

	static FVector4 ExtractVector(int i, std::vector<double> Data) {
		return TVector4(Data[i*4], Data[i*4+1], Data[i*4+2], Data[i*4+3]);
	}

	static FQuat ExtractQuat(int i, std::vector<double> Data) {
		return TQuat(Data[i*4], Data[i*4+1], Data[i*4+2], Data[i*4+3]).GetNormalized();
	}

	static void Orthogonalize(FVector4& Target, const FVector4 Reference) {
		Target -= Reference * (Dot4(Target, Reference) / Reference.SizeSquared());
	}
};

double Parallelness(FVector4 A, FVector4 B) {
	return Dot4(A, B) * Dot4(A, B) / (A.SizeSquared() * B.SizeSquared());
}

bool RunBivectorTestsOnCase(FAutomationTestBase& TestBase, const FTestCase C) {
	FBivector4D AB(C.VA, C.VB); // Most of these identities should hold for general bivectors, not just simple ones (those that can be expressed as a single wedge product). However, correctness for simple bivectors often implies correctness for all of them by linearity.
	FBivector4D CD(C.VC, C.VD);
	FBivector4D EF(C.VE, C.VF);

	// Wedge products
	TestBase.TestEqual(TEXT("wedge product antisymmetry error"), (FBivector4D(C.VA, C.VB) + FBivector4D(C.VB, C.VA)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("wedge product linearity error"), (FBivector4D(C.VA + C.VB * C.S, C.VC) - FBivector4D(C.VA, C.VC) - FBivector4D(C.VB, C.VC) * C.S).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // linearity
	TestBase.TestEqual(TEXT("wedge product size"), FBivector4D(C.VA, C.VPB).Size(), C.VA.Size() * C.VPB.Size(), UNREAL4D_ROTATION_TEST_EPSILON);

	// Bivector-vector products
	TestBase.TestEqual(TEXT("bivector-vector product dot product with the vector"), Dot4(AB * C.VC, C.VC), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector-vector product dot product with the vector"), Dot4(C.VC * AB, C.VC), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("swapped bivector-vector product dot product"), Dot4(AB * C.VC, C.VD), -Dot4(AB * C.VD, C.VC), UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector-vector product linearity error"), (AB * (C.VC + C.VD * C.S) - AB * C.VC - (AB * C.VD) * C.S).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // linearity in the vector
	TestBase.TestEqual(TEXT("bivector-vector product linearity error"), ((AB + CD * C.S) * C.VE - AB * C.VE - (CD * C.VE) * C.S).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // linearity in the bivector
	FVector4 ABC = FBivector4D(C.VA, C.VPB) * C.VC;
	TestBase.TestEqual(TEXT("bivector-vector product parallelness error"), Parallelness(ABC, C.VA) + Parallelness(ABC, C.VPB), 1, UNREAL4D_ROTATION_TEST_EPSILON); // (A^B)*C is in the plane spanned by A and B
	TestBase.TestEqual(TEXT("bivector-vector product orientation error"), Parallelness(FBivector4D(C.VA, C.VPB) * C.VA, C.VPB), 1, UNREAL4D_ROTATION_TEST_BIG_EPSILON); // correct orientation within the plane
	TestBase.TestEqual(TEXT("bivector-vector product size error"), (FBivector4D(C.VA, C.VPB) * C.VA).Size(), C.VA.SizeSquared() * C.VPB.Size(), UNREAL4D_ROTATION_TEST_EPSILON); // overall scale correct

	// Bivector to matrix conversion
	TestBase.TestEqual(TEXT("bivector to matrix conversion error"), (AB.ToMatrix().TransformFVector4(C.VC) - AB * C.VC).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// Bivector-scalar products
	FBivector4D ABS = AB;
	ABS *= C.S;
	TestBase.TestTrue(TEXT("bivector-scalar product matching *="), ABS == AB * C.S); // *= * consistency
	TestBase.TestEqual(TEXT("bivector-scalar division error"), (ABS/C.S - AB).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON); // / inverse of *
	TestBase.TestEqual(TEXT("bivector-scalar product size"), ABS.Size(), AB.Size() * FMath::Abs(C.S), AB.Size() * FMath::Abs(C.S) * UNREAL4D_ROTATION_TEST_EPSILON); // scales correctly
	TestBase.TestTrue(TEXT("bivector AB*(-1)+AB == 0"), (AB * (-1.0) + AB) == FBivector4D());

	// Addition and subtraction are used so extensively in the other tests that it would be kind of hard for an error in them to not get caught elsewhere.

	// Bivector dot products
	TestBase.TestEqual(TEXT("bivector self dot product"), AB | AB, AB.SizeSquared(), UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector perpendicular dot product"), FBivector4D(C.VA, C.VPB) | FBivector4D(C.VPC, C.VPD), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("reversed bivector dot product"), AB | CD, CD | AB, UNREAL4D_ROTATION_TEST_EPSILON); // symmetry
	TestBase.TestEqual(TEXT("bivector dot product linear expandion"), (AB | EF) + (CD | EF) * C.S, (AB + CD * C.S) | EF, UNREAL4D_ROTATION_TEST_EPSILON); // linearity

	// Bivector-bivector products
	TestBase.TestEqual(TEXT("bivector-bivector product linearity error"), ((AB * EF) + (CD * EF) * C.S - ((AB + CD * C.S) * EF)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector-bivector product antisymmetry error"), (AB * CD + CD * AB).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector-bivector product basis error"), (FBivector4D(C.VA, C.VPB) * FBivector4D(C.VPB, C.VPC) - FBivector4D(C.VA, C.VPC) * (C.VPB.SizeSquared()/2)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("Jacobi identity error"), (AB * (CD * EF) + CD * (EF * AB) + EF * (AB * CD)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // Jacobi identity
	TestBase.TestLessThan(TEXT("bivector-bivector product size"), (AB * CD).Size(), (1+UNREAL4D_ROTATION_TEST_EPSILON) * AB.Size() * CD.Size() * UE_DOUBLE_SQRT_2); // this isn't a very precise test because it's an inequality

	// Bivector duals
	TestBase.TestEqual(TEXT("bivector dual linearity error"), ((AB + CD * C.S).Dual() - AB.Dual() - CD.Dual() * C.S).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // linearity
	TestBase.TestEqual(TEXT("bivector dual product with vector in plane"), (AB.Dual() * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("simple bivector dot product with dual"), AB.Dual() | AB, 0, UNREAL4D_ROTATION_TEST_EPSILON); // This applies for simple bivectors only.
	TestBase.TestEqual(TEXT("bivector dual size"), AB.Dual().Size(), AB.Size(), AB.Size() * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector double dual error"), (AB.Dual().Dual() - AB).Size(), 0, 0);
	TestBase.TestEqual(TEXT("bivector dual rotational consistency error"), (AB.Dual() * CD - (AB * CD).Dual()).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // Rotational consistency, infinitessimal version

	// Bivector exponential
	if (AB.Size() < UE_DOUBLE_PI * (1-UNREAL4D_ROTATION_TEST_BIG_EPSILON)) {
		TestBase.TestEqual(TEXT("bivector exp log error"), (AB.Exp().Log() - AB).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON); // log is a partial inverse of exp
	}
	if (AB.Size() < 1 && CD.Size() < 1) {
		TestBase.TestEqual(TEXT("bivector exp Baker-Campbell_Hausdorff approximation error"), (((AB*0.1).Exp() * (CD*0.1).Exp()).Log() - (AB + CD + (AB*CD)*0.05) * 0.1).Size(), 0, 0.001); // Baker-Campbell-Hausdorff formula. This identity is only approximate, having an error cubic in the size of AB and CD. The exact constant for the error is kind of difficult to work out, but I'm confident it's at least less than 1.
	}
	FQuaternionPair AB2Exp = (AB * (1+C.S)).Exp();
	FQuaternionPair ABExp2 = AB.Exp() * (AB * C.S).Exp();
	TestBase.TestEqual(TEXT("bivector exp commutativity error"), AB2Exp.Distance(ABExp2), 0, UNREAL4D_ROTATION_TEST_EPSILON); // exp(X+Y) = exp(X)exp(Y) if X and Y commute (in this case they commute because they're scalar multiples of each other).
	if (AB.Size() < 1) {
		TestBase.TestEqual(TEXT("bivector exp series expansion error"), (AB.Exp() * C.VC - C.VC - AB * C.VC - AB * (AB * C.VC) / 2 - AB * (AB * (AB * C.VC)) / 6).Size(), 0, (0.0517) * C.VC.Size()); // This series expansion is one possible definition of the exponential. The constant in the error term is e-2-1/2-1/6.
	}

	// Decomposition into simple bivectors
	if (C.VA.Size() * C.VPB.Size() > (1+UNREAL4D_ROTATION_TEST_BIG_EPSILON) * C.VPC.Size() * C.VPD.Size()) {
		TestBase.TestEqual(TEXT("bivector planar component error"), ((FBivector4D(C.VA, C.VPB) + FBivector4D(C.VPC, C.VPD)).BiplanarComponent() - FBivector4D(C.VA, C.VPB)).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON); // definition of PlanarComponent
	}
	TestBase.TestEqual(TEXT("bivector planar component planarity error"), (AB+CD).BiplanarComponent().Dual() | (AB+CD).BiplanarComponent(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // the PlanarComponent is indeed a simple bivector
	double LargerEigenvalue;
	double SmallerEigenvalue;
	(AB+CD).Eigenvalues(LargerEigenvalue, SmallerEigenvalue);
	TestBase.TestEqual(TEXT("larger eigenvalue size"), LargerEigenvalue, (AB+CD).BiplanarComponent().Size(), UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("total eigenvalue size"), LargerEigenvalue*LargerEigenvalue + SmallerEigenvalue*SmallerEigenvalue, (AB+CD).SizeSquared(), UNREAL4D_ROTATION_TEST_EPSILON);
	FVector4 Axis = (AB+CD).LargestAxis();
	TestBase.TestEqual(TEXT("bivector main axis size"), Axis.Size(), 1, UNREAL4D_ROTATION_TEST_EPSILON);
	if (LargerEigenvalue > SmallerEigenvalue * (1 + UNREAL4D_ROTATION_TEST_BIG_EPSILON)) { // Otherwise the main plane is not very well defined
		TestBase.TestEqual(TEXT("bivector largest axis in-plane-ness error"), (FBivector4D(Axis, Axis*(AB+CD)) - (AB+CD).BiplanarComponent()).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON);
	}
	TestBase.TestEqual(TEXT("bivector scaling of main axis"), ((AB+CD)*Axis).Size(), LargerEigenvalue, LargerEigenvalue * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector MainPlane error"), ((AB+CD).MainBiplane() * LargerEigenvalue - (AB+CD).BiplanarComponent()).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector MainPlane normalization error"), (AB+CD).MainBiplane().Size(), 1.0, UNREAL4D_ROTATION_TEST_EPSILON);

	// Self-dual and anti-self-dual bivectors are a special case for these functions, so they should be checked separately.
	FBivector4D SelfDual = AB + AB.Dual();
	TestBase.TestEqual(TEXT("bivector MainPlane of self-dual alignment"), SelfDual.MainBiplane() | SelfDual, AB.Size(), AB.Size() * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector MainPlane of self-dual self-dualness"), SelfDual.MainBiplane() | SelfDual.MainBiplane().Dual(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector PlanarComponent of self-dual alignment"), SelfDual.BiplanarComponent() | SelfDual, AB.SizeSquared(), AB.SizeSquared() * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector PlanarComponent of self-dual self-dualness"), SelfDual.BiplanarComponent() | SelfDual.BiplanarComponent().Dual(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	SelfDual.Eigenvalues(LargerEigenvalue, SmallerEigenvalue);
	TestBase.TestEqual(TEXT("bivector PlanarComponent of self-dual size"), SelfDual.BiplanarComponent().Size(), LargerEigenvalue, LargerEigenvalue * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("self-dual bivector eigenvalue"), LargerEigenvalue, SmallerEigenvalue, SmallerEigenvalue * UNREAL4D_ROTATION_TEST_EPSILON); // This holds only for self-dual and anti-self-dual bivectors.

	FBivector4D AntiSelfDual = AB - AB.Dual();
	TestBase.TestEqual(TEXT("bivector MainPlane of anti-self-dual alignment"), AntiSelfDual.MainBiplane() | AntiSelfDual, AB.Size(), AB.Size() * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector MainPlane of anti-self-dual self-dualness"), AntiSelfDual.MainBiplane() | AntiSelfDual.MainBiplane().Dual(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector PlanarComponent of anti-self-dual alignment"), AntiSelfDual.BiplanarComponent() | AntiSelfDual, AB.SizeSquared(), AB.SizeSquared() * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("bivector PlanarComponent of anti-self-dual self-dualness"), AntiSelfDual.BiplanarComponent() | AntiSelfDual.BiplanarComponent().Dual(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	AntiSelfDual.Eigenvalues(LargerEigenvalue, SmallerEigenvalue);
	TestBase.TestEqual(TEXT("bivector PlanarComponent of anti-self-dual size"), AntiSelfDual.BiplanarComponent().Size(), LargerEigenvalue, LargerEigenvalue * UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("anti-self-dual bivector eigenvalue"), LargerEigenvalue, SmallerEigenvalue, SmallerEigenvalue * UNREAL4D_ROTATION_TEST_EPSILON); // This holds only for self-dual and anti-self-dual bivectors.

	// Bivector size
	TestBase.TestEqual(TEXT("size of sum of perpendicular bivectors"), (FBivector4D(C.VA, C.VPB) + FBivector4D(C.VPC, C.VPD)).SizeSquared(), FBivector4D(C.VA, C.VPB).SizeSquared() + FBivector4D(C.VPC, C.VPD).SizeSquared(), UNREAL4D_ROTATION_TEST_EPSILON);

	// Bivector normalization
	FBivector4D ABN = AB;
	ABN.Normalize();
	TestBase.TestEqual(TEXT("bivector normalize vs. normalized error"), (ABN - AB.Normalized()).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON);
	TestBase.TestEqual(TEXT("bivector normalize idempotency error"), (ABN - ABN.Normalized()).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("scaled bivector normalize error"), (ABN - (AB * FMath::Abs(C.S)).Normalized()).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON); // scaling doesn't affect the normalized result.
	TestBase.TestEqual(TEXT("bivector normalize coplanarity error"), (ABN.Dual() * AB).Size(), 0, UNREAL4D_ROTATION_TEST_BIG_EPSILON); // a rough test for ABN and AB being coplanar.
	return true;
}

void InPlaneTest(FAutomationTestBase& TestBase, FQuaternionPair AB, double Angle, FVector4 A, FVector4 B) {
	TestBase.TestEqual(TEXT("single-plane QuatPair error"), (AB * A - A * FMath::Cos(Angle) - B * FMath::Sin(Angle)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	FVector4 Perpendicular = FBivector4D(A, B).Dual() * FVector4(0.5,0.7,1,1.1);
	TestBase.TestEqual(TEXT("single-plane QuatPair planarity error"), (AB * Perpendicular - Perpendicular).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
}

bool RunQuatPairTestsOnCase(FAutomationTestBase& TestBase, const FTestCase C) {
	// vector rotation
	TestBase.TestEqual(TEXT("QuatPair identity error"), (FQuaternionPair::Identity * C.VA - C.VA).Size(), 0, 0);
	TestBase.TestEqual(TEXT("QuatPair vector rotation resulting size"), (C.QA * C.VA).Size(), C.VA.Size(), UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair vector rotation linearity error"), (C.S * (C.QA * C.VA) + C.QA * C.VB - C.QA * (C.VA * C.S + C.VB)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// 3D rotation
	TQuat Q3D = C.QA.Left;
	TestBase.TestEqual(TEXT("QuatPair from 3D rotation error"), (TVector4(Q3D * TVector(C.VA), C.VA.W) - FQuaternionPair(Q3D) * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// single-plane rotations
	InPlaneTest(TestBase, FQuaternionPair::FromXY(C.S), C.S, FVector4(1,0,0,0), FVector4(0,1,0,0));
	InPlaneTest(TestBase, FQuaternionPair::FromXZ(C.S), C.S, FVector4(1,0,0,0), FVector4(0,0,1,0));
	InPlaneTest(TestBase, FQuaternionPair::FromXW(C.S), C.S, FVector4(1,0,0,0), FVector4(0,0,0,1));
	InPlaneTest(TestBase, FQuaternionPair::FromYX(C.S), C.S, FVector4(0,1,0,0), FVector4(1,0,0,0));
	InPlaneTest(TestBase, FQuaternionPair::FromYZ(C.S), C.S, FVector4(0,1,0,0), FVector4(0,0,1,0));
	InPlaneTest(TestBase, FQuaternionPair::FromYW(C.S), C.S, FVector4(0,1,0,0), FVector4(0,0,0,1));
	InPlaneTest(TestBase, FQuaternionPair::FromZX(C.S), C.S, FVector4(0,0,1,0), FVector4(1,0,0,0));
	InPlaneTest(TestBase, FQuaternionPair::FromZY(C.S), C.S, FVector4(0,0,1,0), FVector4(0,1,0,0));
	InPlaneTest(TestBase, FQuaternionPair::FromZW(C.S), C.S, FVector4(0,0,1,0), FVector4(0,0,0,1));
	InPlaneTest(TestBase, FQuaternionPair::FromWX(C.S), C.S, FVector4(0,0,0,1), FVector4(1,0,0,0));
	InPlaneTest(TestBase, FQuaternionPair::FromWY(C.S), C.S, FVector4(0,0,0,1), FVector4(0,1,0,0));
	InPlaneTest(TestBase, FQuaternionPair::FromWZ(C.S), C.S, FVector4(0,0,0,1), FVector4(0,0,1,0));

	// composition
	TestBase.TestEqual(TEXT("QuatPair composition error"), ((C.QA * C.QB) * C.VA - C.QA * (C.QB * C.VA)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	FQuaternionPair QX = C.QA;
	QX *= C.QB;
	TestBase.TestEqual(TEXT("QuatPair *= error"), QX.Distance(C.QA * C.QB), 0, 0);

	// size and normalization
	QX = FQuaternionPair(C.QA.Left + C.QB.Left, C.QA.Right + C.QB.Right);
	TestBase.TestEqual(TEXT("non-normalized QuatPair result size"), (QX * C.VA).Size(), QX.ScaleFactor() * C.VA.Size(), UNREAL4D_ROTATION_TEST_EPSILON);
	FQuaternionPair QXN = QX;
	QXN.Normalize();
	TestBase.TestEqual(TEXT("QuatPair Normalize vs. GetNormalized error"), QXN.Distance(QX.GetNormalized()), 0, 0);
	TestBase.TestEqual(TEXT("QuatPair normalize result size"), QXN.Size(), UE_DOUBLE_SQRT_2, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair normalize result scale"), QXN.ScaleFactor(), 1, UNREAL4D_ROTATION_TEST_EPSILON);

	// dot product. The QuatPair dot product isn't really related to much, so it lacks suitable qualities to test.
	TestBase.TestEqual(TEXT("QuatPair dot product symmetry error"), (C.QA | C.QB) - (C.QB | C.QA), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// inverse
	TestBase.TestEqual(TEXT("QuatPair inverse error"), (C.QA * C.QA.Inverse()).Distance(FQuaternionPair::Identity), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// decomposition into planes of rotation.
	FBivector4D MainPlane;
	double MainAngle;
	FBivector4D MinorPlane;
	double MinorAngle;
	C.QA.ToPlanesAndAngles(MainPlane, MainAngle, MinorPlane, MinorAngle);
	FVector4 MainPlaneAxis = MainPlane.LargestAxis();
	FVector4 MainPlaneOtherAxis = MainPlane * MainPlaneAxis;
	FVector4 MinorPlaneAxis = MinorPlane.LargestAxis();
	FVector4 MinorPlaneOtherAxis = MinorPlane * MinorPlaneAxis;
	TestBase.TestEqual(TEXT("QuatPair plane perpendicularity error 1"), Dot4(MainPlaneAxis, MinorPlaneAxis), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair plane perpendicularity error 2"), Dot4(MainPlaneAxis, MinorPlaneOtherAxis), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair plane perpendicularity error 3"), Dot4(MainPlaneOtherAxis, MinorPlaneAxis), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair plane perpendicularity error 4"), Dot4(MainPlaneOtherAxis, MinorPlaneOtherAxis), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair main plane rotation error"), (C.QA * MainPlaneAxis - MainPlaneAxis * FMath::Cos(MainAngle) - MainPlaneOtherAxis * FMath::Sin(MainAngle)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair minor plane rotation error"), (C.QA * MinorPlaneAxis - MinorPlaneAxis * FMath::Cos(MinorAngle) - MinorPlaneOtherAxis * FMath::Sin(MinorAngle)).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestTrue(TEXT("QuatPair decomposed angles positive"), MainAngle >= 0 && MinorAngle >= 0);
	TestBase.TestGreaterThan(TEXT("QuatPair main angle"), MainAngle, MinorAngle, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestLessThan(TEXT("QuatPair main angle"), MainAngle, UE_DOUBLE_PI, UNREAL4D_ROTATION_TEST_EPSILON);

	// slerp
	TestBase.TestEqual(TEXT("QuatPair slerp start error"), (FQuaternionPair::Slerp(C.QA, C.QB, 0) * C.VA - C.QA * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair slerp end error"),   (FQuaternionPair::Slerp(C.QA, C.QB, 1) * C.VA - C.QB * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON); // Because QuatPair double-covers SO(4), the result here may be equivalent but not equal.
	double S = FMath::Fmod(C.S, 1);
	TestBase.TestEqual(TEXT("QuatPair slerp right rotational consistency error"), FQuaternionPair::Slerp(C.QA * C.QC, C.QB * C.QC, S).Distance(FQuaternionPair::Slerp(C.QA, C.QB, S) * C.QC), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair slerp left rotational consistency error"),  FQuaternionPair::Slerp(C.QC * C.QA, C.QC * C.QB, S).Distance(C.QC * FQuaternionPair::Slerp(C.QA, C.QB, S)), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair slerp error"), FQuaternionPair::Slerp(FQuaternionPair::Identity, C.QA, S).Distance(FQuaternionPair(MainPlane, MainAngle * S) * FQuaternionPair(MinorPlane, MinorAngle * S)), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// QuatPair to matrix conversion
	TestBase.TestEqual(TEXT("QuatPair to matrix error"), (C.QA.ToMatrix().TransformFVector4(C.VA) - C.QA * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair to matrix and back error"), (FQuaternionPair(C.QA.ToMatrix()) * C.VA - C.QA * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	// Euler4D conversions
	TestBase.TestEqual(TEXT("QuatPair to Euler4D error"), (FEuler4D::RadiansFromQuatPair(C.QA).RotatePoint(C.VA) - C.QA * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);
	TestBase.TestEqual(TEXT("QuatPair to Euler4D and back error"), (FEuler4D::RadiansFromQuatPair(C.QA).RadiansToQuatPair() * C.VA - C.QA * C.VA).Size(), 0, UNREAL4D_ROTATION_TEST_EPSILON);

	return true;
}

bool FUnreal4D_RotationTests::RunTest(const FString& _Parameters) {
	int TestIndex = 0;
	std::uniform_real_distribution<double> Dist(-1.5, 1.5);
	std::minstd_rand Rand; // Initialize a fixed RNG with a fixed seed (the default, 1)
	while (TestIndex < 100) {
		std::vector<double> Data(FTestCase::REQUIRED_DATA);
		for (int i=0; i<Data.size(); i++) {
			Data[i] = Dist(Rand);
		}
		FTestCase TestCase(Data);
		if (TestCase.IsSuitable(UNREAL4D_ROTATION_TEST_BIG_EPSILON)) {
			RunBivectorTestsOnCase(*this, TestCase);
			RunQuatPairTestsOnCase(*this, TestCase);
			TestIndex++;
		}
	}

	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
