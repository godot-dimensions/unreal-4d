#pragma once

#include "../Unreal4DDefines.h"
#include "Math/Vector4.h"
#include "Containers/Array.h"

#include "Geometry4D.generated.h"

// Static helper class for misc 4D geometry functions like line intersections.
UCLASS()
class UNREAL4DCORE_API UGeometry4D : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Barycentric tetrahedra calculations. Don't expose these with UFUNCTION, it just needs to be
	// efficient and shared between UTetraMesh4D, UConcaveMeshShape4D, UConvexHullShape4D, etc.
	static bool ComputeInverseMetric3x3(const double InG00, const double InG01, const double InG02, const double InG11, const double InG12, const double InG22, double OutInvSymmetric[6]);
	static void GetNearestPointOnTetrahedronBarycentric(const FVector4& InVert0, const FVector4& InVert1, const FVector4& InVert2, const FVector4& InVert3, const FVector4& InPoint, const TArray<double>& InNearestTetraInverseGramMetricCache, const int64 InTetrahedronIndex, FVector4& OutNearestOnTet, double& OutDistanceSquared, bool& OutProjInside);
	static bool IsPointInsideTetrahedronBarycentric(const FVector4& InVert0, const FVector4& InVert1, const FVector4& InVert2, const FVector4& InVert3, const FVector4& InPoint, const TArray<double>& InNearestTetraInverseGramMetricCache, const int64 InTetrahedronIndex);

	// Point-line calculations.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static FVector4 ClosestPointOnLine(const FVector4& InLinePosition, const FVector4& InLineDirection, const FVector4& InPoint);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static FVector4 ClosestPointOnLineSegment(const FVector4& InLineA, const FVector4& InLineB, const FVector4& InPoint);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static FVector4 ClosestPointOnRay(const FVector4& InRayOrigin, const FVector4& InRayDirection, const FVector4& InPoint);

	// Point-shape calculations.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static FVector4 ClosestPointOnTriangle(const FVector4& InTriangleA, const FVector4& InTriangleB, const FVector4& InTriangleC, const FVector4& InPoint);

	// Line-line calculations.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static FVector4 ClosestPointBetweenLines(const FVector4& InLine1Point, const FVector4& InLine1Dir, const FVector4& InLine2Point, const FVector4& InLine2Dir);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static FVector4 ClosestPointBetweenLineSegments(const FVector4& InLine1A, const FVector4& InLine1B, const FVector4& InLine2A, const FVector4& InLine2B);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static TArray<FVector4> ClosestPointsBetweenLines(const FVector4& InLine1Point, const FVector4& InLine1Dir, const FVector4& InLine2Point, const FVector4& InLine2Dir);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static TArray<FVector4> ClosestPointsBetweenLineSegments(const FVector4& InLine1A, const FVector4& InLine1B, const FVector4& InLine2A, const FVector4& InLine2B);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static TArray<FVector4> ClosestPointsBetweenLineAndSegment(const FVector4& InLinePoint, const FVector4& InLineDirection, const FVector4& InSegmentA, const FVector4& InSegmentB);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static double DistanceBetweenLineAndSegment(const FVector4& InLinePoint, const FVector4& InLineDirection, const FVector4& InSegmentA, const FVector4& InSegmentB);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Geometry")
	static bool BoxContains(const FVector4& InBoxOrigin, const FVector4& InBoxExtents, const FVector4& InPoint);
};
