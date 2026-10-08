#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "../../../../Unreal4DCore/Math/Basis4D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(UUnreal4D_Basis4D_PrecomputedInversion, "Unreal4D.Basis4D.PrecomputedInversion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool UUnreal4D_Basis4D_PrecomputedInversion::RunTest(const FString& Parameters)
{
	const FMatrix ones = UBasis4D::Basis4D_FromNumbers(1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, 1);
	const FMatrix onesAdjugate = UBasis4D::Basis4D_FromNumbers(-4, -4, -4, 4, -4, -4, 4, -4, -4, 4, -4, -4, 4, -4, -4, -4);
	const FMatrix onesInverse = UBasis4D::Basis4D_FromNumbers(0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, 0.25);
	TestTrue(TEXT("Adjugate should match precomputed."), UBasis4D::Basis4D_Adjugate(ones).Equals(onesAdjugate, UNREAL4D_COMPARISON_EPSILON));
	TestTrue(TEXT("Inverse should match precomputed."), UBasis4D::Basis4D_Inverse(ones).Equals(onesInverse, UNREAL4D_COMPARISON_EPSILON));

	const FMatrix unique = UBasis4D::Basis4D_FromNumbers(1.0, 10.41, 0.31, 0.21, 0.11, 1.1, 0.12, 0.22, 0.32, 0.42, 1.2, 0.43, 0.33, 0.23, 0.13, 1.3);
	const FMatrix uniqueInverse = UBasis4D::Basis4D_FromNumbers(1.131305786134016, -0.31456736720963524, -0.2559375714833072, -0.04485864504938581, -0.03978939804196893, 0.9801066566654912, -0.07307806077708652, -0.13526471134111334, -0.19433619846736885, -0.23401553809120415, 0.9326678341313813, -0.23750242201406277, -0.2607043400567804, -0.07015021530925138, -0.015368819899121764, 0.8282996548742938);
	TestTrue(TEXT("Unique inverse should match precomputed."), UBasis4D::Basis4D_Inverse(unique).Equals(uniqueInverse, UNREAL4D_COMPARISON_EPSILON));
	TestTrue(TEXT("Inverse of inverse should be original."), UBasis4D::Basis4D_Inverse(uniqueInverse).Equals(unique, UNREAL4D_COMPARISON_EPSILON));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
