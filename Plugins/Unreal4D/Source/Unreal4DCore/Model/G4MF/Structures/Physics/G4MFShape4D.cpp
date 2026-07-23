#include "G4MFShape4D.h"

#include "../../../../Physics/Shapes/Components/BoxShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/CapsuleShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/ConcaveMeshShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/ConvexHullShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/CubinderShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/CylinderShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/DuocylinderShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/OrthoplexShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/PlaneShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/RayShape4DComponent.h"
#include "../../../../Physics/Shapes/Components/SphereShape4DComponent.h"
#include "../../../../Physics/Shapes/Data/BoxShape4DData.h"
#include "../../../../Physics/Shapes/Data/CapsuleShape4DData.h"
#include "../../../../Physics/Shapes/Data/ConcaveMeshShape4DData.h"
#include "../../../../Physics/Shapes/Data/ConvexHullShape4DData.h"
#include "../../../../Physics/Shapes/Data/CubinderShape4DData.h"
#include "../../../../Physics/Shapes/Data/CylinderShape4DData.h"
#include "../../../../Physics/Shapes/Data/DuocylinderShape4DData.h"
#include "../../../../Physics/Shapes/Data/OrthoplexShape4DData.h"
#include "../../../../Physics/Shapes/Data/PlaneShape4DData.h"
#include "../../../../Physics/Shapes/Data/RayShape4DData.h"
#include "../../../../Physics/Shapes/Data/SphereShape4DData.h"
#include "../../G4MFState4D.h"
#include "../G4MFAccessor4D.h"

bool UG4MFShape4D::IsEqualExact(const UG4MFShape4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	if (ShapeType != InOther->ShapeType)
	{
		return false;
	}
	if (BaseSize != InOther->BaseSize)
	{
		return false;
	}
	if (MeshIndex != InOther->MeshIndex)
	{
		return false;
	}
	if (RayLength != InOther->RayLength)
	{
		return false;
	}
	// HeightMap properties.
	if (GridSpacing != InOther->GridSpacing)
	{
		return false;
	}
	if (GridSize != InOther->GridSize)
	{
		return false;
	}
	if (HeightsAccessorIndex != InOther->HeightsAccessorIndex)
	{
		return false;
	}
	// Curves.
	if (Curves.Num() != InOther->Curves.Num())
	{
		return false;
	}
	for (int32 i = 0; i < Curves.Num(); ++i)
	{
		if (!IsValid(Curves[i]) || !IsValid(InOther->Curves[i]) || !Curves[i]->IsEqualExact(InOther->Curves[i]))
		{
			return false;
		}
	}
	return true;
}

/*
Ref<Shape4D> G4MFShape4D::_generate_shape_from_general() const {
	Ref<Shape4D> ret;
	if (_curves.is_empty()) {
		// No curves? This is just a box shape.
		Ref<BoxShape4D> box_shape;
		box_shape.instantiate();
		box_shape->set_size(_base_size);
		return box_shape;
	} else if (_curves.size() == 1) {
		const Ref<GeneralShapeCurve4D> curve = _curves[0];
		const FVector4 curve_radii = curve->get_radii();
		const int curve_dimension = curve->get_radii_dimension(); // Amount of non-zero radii.
		const double curve_radius = curve->get_radii_sum() / curve_dimension; // Average radius of the non-zero radii.
		if (Math::is_zero_approx(_base_size.x) && Math::is_zero_approx(_base_size.z)) {
			// A single curve with no X or Z base size? Could be one of many 4D shapes: sphere, capsule, cylinder, cubinder, orthoplex.
			if (curve->get_exponent() == 1.0 && Math::is_zero_approx(_base_size.y) && Math::is_zero_approx(_base_size.w)) {
				// If the exponent is 1.0 and there is no base size, it is an orthoplex.
				Ref<OrthoplexShape4D> orthoplex_shape;
				orthoplex_shape.instantiate();
				orthoplex_shape->set_half_extents(curve_radii);
				return orthoplex_shape;
			} else if (curve_radii.x == curve_radius && curve_radii.z == curve_radius) {
				// The curve is uniform on X and Z? Could be one of many 4D shapes: sphere, capsule, cylinder, cubinder.
				if (curve_dimension == 4) {
					// A shape with one curve of dimension 4 can be a 4D sphere or a 4D capsule (or not one of our special shapes).
					if (Math::is_zero_approx(_base_size.w) && curve_radii.y == curve_radius && curve_radii.w == curve_radius) {
						// It passed! Now, one last check. The only difference between these two is that a sphere has no mid-height.
						if (Math::is_zero_approx(_base_size.y)) {
							Ref<SphereShape4D> sphere_shape;
							sphere_shape.instantiate();
							sphere_shape->set_radius(curve_radius);
							return sphere_shape;
						} else {
							Ref<CapsuleShape4D> capsule_shape;
							capsule_shape.instantiate();
							capsule_shape->set_radius(curve_radius);
							capsule_shape->set_height(_base_size.y);
							return capsule_shape;
						}
					}
				} else if (curve_dimension == 3) {
					// A shape with one curve of dimension 3 can be a 4D cylinder / spherinder (or not one of our special shapes).
					if (Math::is_zero_approx(_base_size.w) && Math::is_zero_approx(curve_radii.y) && curve_radii.w == curve_radius) {
						Ref<CylinderShape4D> cylinder_shape;
						cylinder_shape.instantiate();
						cylinder_shape->set_radius(curve_radius);
						cylinder_shape->set_height(_base_size.y);
						return cylinder_shape;
					}
				} else if (curve_dimension == 2) {
					// A shape with one curve of dimension 2 can be a 4D cubinder (or not one of our special shapes).
					// All the previous checks are actually sufficient to narrow it down to a cubinder.
					Ref<CubinderShape4D> cubinder_shape;
					cubinder_shape.instantiate();
					cubinder_shape->set_radius(curve_radius);
					cubinder_shape->set_height(_base_size.y);
					cubinder_shape->set_thickness(_base_size.w);
					return cubinder_shape;
				}
			}
		}
	} else if (_curves.size() == 2 && _base_size.is_zero_approx()) {
		// The only special case shape with two curves is a duocylinder.
		const Ref<GeneralShapeCurve4D> curve0 = _curves[0];
		const Ref<GeneralShapeCurve4D> curve1 = _curves[1];
		const FVector4 curve0_radii = curve0->get_radii();
		const FVector4 curve1_radii = curve1->get_radii();
		if (curve0_radii.x == curve0_radii.y && curve0_radii.z == 0.0f && curve0_radii.w == 0.0f && curve1_radii.x == 0.0f && curve1_radii.y == 0.0f && curve1_radii.z == curve1_radii.w) {
			Ref<DuocylinderShape4D> duocylinder_shape;
			duocylinder_shape.instantiate();
			duocylinder_shape->set_radius_xy(curve0_radii.x);
			duocylinder_shape->set_radius_zw(curve1_radii.z);
			return duocylinder_shape;
		}
	}
	// If none of the special case shape types match, return a GeneralShape4D.
	Ref<GeneralShape4D> general_shape;
	general_shape.instantiate();
	general_shape->set_base_size(_base_size);
	general_shape->set_curves(_curves.duplicate());
	return general_shape;
}

Ref<Shape4D> G4MFShape4D::generate_shape(const Ref<G4MFState4D> &p_g4mf_state) const {
	Ref<Shape4D> ret;
	if (_shape_type == "concave") {
		Ref<ConcaveMeshShape4D> concave_shape;
		concave_shape.instantiate();
		ERR_PRINT("G4MFShape4D.generate_shape: Godot 4D does not support concave mesh shapes yet.");
		ret = concave_shape;
	} else if (_shape_type == "convex") {
		Ref<ConvexHullShape4D> convex_shape;
		convex_shape.instantiate();
		ERR_PRINT("G4MFShape4D.generate_shape: Godot 4D does not support convex hull mesh shapes yet.");
		ret = convex_shape;
	} else if (_shape_type == "heightmap") {
		Ref<HeightMapShape4D> heightmap_shape;
		heightmap_shape.instantiate();
		heightmap_shape->set_grid_size(_grid_size);
		heightmap_shape->set_grid_spacing(_grid_spacing);
		heightmap_shape->set_height_data(load_heights(p_g4mf_state));
		ret = heightmap_shape;
	} else if (_shape_type == "plane") {
		Ref<PlaneShape4D> plane_shape;
		plane_shape.instantiate();
		// PlaneShape4D has no data to set.
		ret = plane_shape;
	} else if (_shape_type == "ray") {
		Ref<RayShape4D> ray_shape;
		ray_shape.instantiate();
		ray_shape->set_ray_length(_ray_length);
		ret = ray_shape;
	} else {
		// Either this is a general shape or we don't know what it is, so generate a general shape.
		ret = _generate_shape_from_general();
	}
	ERR_FAIL_COND_V_MSG(ret.is_null(), ret, "Failed to generate shape from G4MFShape4D.");
	ret->set_name(get_name());
	return ret;
}
*/

UG4MFShape4D* UG4MFShape4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFShape4D* ret = NewObject<UG4MFShape4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFShape4D-specific code.
	if (InJson->HasField(TEXT("type")))
	{
		ret->ShapeType = InJson->GetStringField(TEXT("type"));
	}

	if (InJson->HasField(TEXT("size")))
	{
		TArray<TSharedPtr<FJsonValue>> baseSizeArray = InJson->GetArrayField(TEXT("size"));
		ret->BaseSize = UVectorFuncs4D::Vector4_FromJsonArray(baseSizeArray);
	}

	if (InJson->HasField(TEXT("length")))
	{
		ret->RayLength = InJson->GetNumberField(TEXT("length"));
	}

	if (InJson->HasField(TEXT("mesh")))
	{
		ret->MeshIndex = InJson->GetIntegerField(TEXT("mesh"));
	}
	// HeightMap properties.
	if (InJson->HasField(TEXT("gridSize")))
	{
		TArray<TSharedPtr<FJsonValue>> gridSizeArray = InJson->GetArrayField(TEXT("gridSize"));
		ret->GridSize = UG4MFItem4D::JsonArrayToVector3i(gridSizeArray);
	}
	if (InJson->HasField(TEXT("gridSpacing")))
	{
		TArray<TSharedPtr<FJsonValue>> gridSpacingArray = InJson->GetArrayField(TEXT("gridSpacing"));
		ret->GridSpacing = UG4MFItem4D::JsonArrayToVector3(gridSpacingArray);
	}
	if (InJson->HasField(TEXT("heights")))
	{
		ret->HeightsAccessorIndex = InJson->GetIntegerField(TEXT("heights"));
	}
	// Curves.
	if (InJson->HasField(TEXT("curves")))
	{
		TArray<TSharedPtr<FJsonValue>> curvesJsonArray = InJson->GetArrayField(TEXT("curves"));
		for (const TSharedPtr<FJsonValue>& curveJsonValue : curvesJsonArray)
		{
			const TSharedPtr<FJsonObject>& curveJsonObject = curveJsonValue->AsObject();
			if (!curveJsonObject.IsValid())
			{
				continue;
			}
			UGeneralShapeCurve4D* curve = NewObject<UGeneralShapeCurve4D>(InG4MFState);
			if (curveJsonObject->HasField(TEXT("radii")))
			{
				TArray<TSharedPtr<FJsonValue>> radiiJsonArray = curveJsonObject->GetArrayField(TEXT("radii"));
				curve->Radii = UVectorFuncs4D::Vector4_FromJsonArray(radiiJsonArray);
			}
			if (curveJsonObject->HasField(TEXT("exponent")))
			{
				curve->Exponent = curveJsonObject->GetNumberField(TEXT("exponent"));
			}
			if (curveJsonObject->HasField(TEXT("taper")))
			{
				TArray<TSharedPtr<FJsonValue>> taperJsonArray = curveJsonObject->GetArrayField(TEXT("taper"));
				for (const TSharedPtr<FJsonValue>& taperJsonValue : taperJsonArray)
				{
					const TSharedPtr<FJsonObject>& taperJsonObject = taperJsonValue->AsObject();
					if (!taperJsonObject.IsValid())
					{
						continue;
					}
					FGeneralShapeCurveTaperPoint4D taperPoint = FGeneralShapeCurveTaperPoint4D();
					if (taperJsonObject->HasField(TEXT("position")))
					{
						TArray<TSharedPtr<FJsonValue>> positionJsonArray = taperJsonObject->GetArrayField(TEXT("position"));
						taperPoint.Position = UVectorFuncs4D::Vector4_FromJsonArray(positionJsonArray);
					}
					if (taperJsonObject->HasField(TEXT("radii")))
					{
						TArray<TSharedPtr<FJsonValue>> radiiJsonArray = taperJsonObject->GetArrayField(TEXT("radii"));
						taperPoint.Radii = UVectorFuncs4D::Vector4_FromJsonArray(radiiJsonArray);
					}
					if (taperJsonObject->HasField(TEXT("exponent")))
					{
						taperPoint.Exponent = taperJsonObject->GetNumberField(TEXT("exponent"));
					}
					curve->Taper.Add(taperPoint);
				}
			}
			ret->Curves.Add(curve);
		}
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFShape4D::ToJson() const
{
	TSharedPtr<FJsonObject> ret = UG4MFItem4D::WriteItemEntriesToJson();
	if (ShapeType != TEXT("general"))
	{
		ret->SetStringField(TEXT("type"), ShapeType);
	}
	if (!BaseSize.Equals(FVector4(0.0, 0.0, 0.0, 0.0), UNREAL4D_COMPARISON_EPSILON))
	{
		ret->SetArrayField(TEXT("size"), UVectorFuncs4D::Vector4_ToJsonArray(BaseSize));
	}
	if (RayLength != 1.0)
	{
		ret->SetNumberField(TEXT("length"), RayLength);
	}
	if (MeshIndex > -1)
	{
		ret->SetNumberField(TEXT("mesh"), MeshIndex);
	}
	// HeightMap properties.
	if (GridSize != FIntVector(1, 1, 1))
	{
		ret->SetArrayField(TEXT("gridSize"), UG4MFItem4D::Vector3iToJsonArray(GridSize));
	}
	if (GridSpacing != FVector(1.0, 1.0, 1.0))
	{
		ret->SetArrayField(TEXT("gridSpacing"), UG4MFItem4D::Vector3ToJsonArray(GridSpacing));
	}
	if (HeightsAccessorIndex > -1)
	{
		ret->SetNumberField(TEXT("heights"), HeightsAccessorIndex);
	}
	// Curves.
	if (Curves.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> curvesJsonArray;
		for (const TObjectPtr<UGeneralShapeCurve4D>& curve : Curves)
		{
			TSharedPtr<FJsonObject> curveJsonObject = MakeShared<FJsonObject>();
			if (!curve->Radii.Equals(FVector4(0.0, 0.0, 0.0, 0.0), UNREAL4D_COMPARISON_EPSILON))
			{
				curveJsonObject->SetArrayField(TEXT("radii"), UVectorFuncs4D::Vector4_ToJsonArray(curve->Radii));
			}
			if (curve->Exponent != 2.0)
			{
				curveJsonObject->SetNumberField(TEXT("exponent"), curve->Exponent);
			}
			if (curve->Taper.Num() > 0)
			{
				TArray<TSharedPtr<FJsonValue>> taperJsonArray;
				for (const FGeneralShapeCurveTaperPoint4D& taperPoint : curve->Taper)
				{
					TSharedPtr<FJsonObject> taperJsonObject = MakeShared<FJsonObject>();
					if (!taperPoint.Position.Equals(FVector4(0.0, 0.0, 0.0, 0.0), UNREAL4D_COMPARISON_EPSILON))
					{
						taperJsonObject->SetArrayField(TEXT("position"), UVectorFuncs4D::Vector4_ToJsonArray(taperPoint.Position));
					}
					if (!taperPoint.Radii.Equals(FVector4(0.0, 0.0, 0.0, 0.0), UNREAL4D_COMPARISON_EPSILON))
					{
						taperJsonObject->SetArrayField(TEXT("radii"), UVectorFuncs4D::Vector4_ToJsonArray(taperPoint.Radii));
					}
					if (taperPoint.Exponent != 2.0)
					{
						taperJsonObject->SetNumberField(TEXT("exponent"), taperPoint.Exponent);
					}
					taperJsonArray.Add(MakeShared<FJsonValueObject>(taperJsonObject));
				}
				curveJsonObject->SetArrayField(TEXT("taper"), taperJsonArray);
			}
			curvesJsonArray.Add(MakeShared<FJsonValueObject>(curveJsonObject));
		}
		ret->SetArrayField(TEXT("curves"), curvesJsonArray);
	}
	return ret;
}
