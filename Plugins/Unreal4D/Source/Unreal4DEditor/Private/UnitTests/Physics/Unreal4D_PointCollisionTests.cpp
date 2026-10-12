#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../../../../Unreal4DCore/Physics/Server/PhysicsServer4D.h"
#include "../../../../Unreal4DCore/Scene/Components/Mesh4DComponent.h"
#include "../../../../Unreal4DCore/Model/Mesh/Tetra/BoxTetraMesh4D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_PointCollision, "Unreal4D.Physics.PointCollision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_PointCollision::RunTest(const FString& Parameters)
{
	UMesh4DComponent* collider = NewObject<UMesh4DComponent>();
	UBoxTetraMesh4D* mesh = NewObject<UBoxTetraMesh4D>(collider);
	mesh->SetSize(FVector4(2.0, 2.0, 2.0, 2.0));
	collider->Mesh4D = mesh;
	FCollisionManifold4D collision;
	const FVector4 insidePoint(0.75, 0.0, 0.0, 0.0);
	TestFalse(TEXT("Collision is disabled by default."), collider->QueryPointCollision(insidePoint, collision));
	collider->EnableCollision = true;
	TestTrue(TEXT("Interior point hits a plain Unreal4D mesh without Chaos state."), collider->QueryPointCollision(insidePoint, collision));
	TestTrue(TEXT("Hit references the original mesh component."), collision.ColliderPrimitive == collider && collision.IsValid());
	TestTrue(TEXT("Surface point is on the nearest box face."), collision.Point.Equals(FVector4(1.0, 0.0, 0.0, 0.0), 1e-6));
	TestTrue(TEXT("Normal points out of the mesh."), collision.Normal.Equals(FVector4(1.0, 0.0, 0.0, 0.0), 1e-6));
	TestTrue(TEXT("Signed penetration is preserved."), FMath::IsNearlyEqual(collision.Penetration, -0.25f));
	TestFalse(TEXT("Exterior XYZ point misses."), collider->QueryPointCollision(FVector4(2.0, 0.0, 0.0, 0.0), collision));
	TestFalse(TEXT("Exterior W point misses."), collider->QueryPointCollision(FVector4(0.0, 0.0, 0.0, 2.0), collision));
	TestTrue(TEXT("Boundary point hits."), collider->QueryPointCollision(FVector4(1.0, 0.0, 0.0, 0.0), collision));
	collider->SetGlobalPosition4D(FVector4(10.0, 20.0, 30.0, 40.0));
	const FVector4 worldInside = collider->GetGlobalTransform4D().TransformPosition(insidePoint);
	TestTrue(TEXT("Query uses all four world coordinates."), collider->QueryPointCollision(worldInside, collision));
	TestTrue(TEXT("Surface point is returned in world space."), collision.Point.Equals(FVector4(11.0, 20.0, 30.0, 40.0), 1e-6));
	TestFalse(TEXT("An untranslated point misses a translated mesh."), collider->QueryPointCollision(insidePoint, collision));
	UBoxTetraMesh4D* smallerMesh = NewObject<UBoxTetraMesh4D>(collider);
	smallerMesh->SetSize(FVector4(1.0, 1.0, 1.0, 1.0));
	collider->CollisionMesh4D = smallerMesh;
	TestFalse(TEXT("Collision mesh override replaces the rendered mesh for queries."), collider->QueryPointCollision(worldInside, collision));
	collider->CollisionMesh4D = nullptr;
	UPhysicsServer4DWorldSubsystem* server = NewObject<UPhysicsServer4DWorldSubsystem>();
	server->RegisterCollider(collider);
	server->RegisterCollider(collider);
	TArray<FCollisionManifold4D> collisions;
	TestTrue(TEXT("Server dispatches registered collider queries."), server->QueryPointCollision(worldInside, collisions));
	TestEqual(TEXT("Duplicate registration produces one hit."), collisions.Num(), 1);
	server->UnregisterCollider(collider);
	collisions.Reset();
	TestFalse(TEXT("Unregistered colliders are excluded."), server->QueryPointCollision(worldInside, collisions));
	UMesh4DComponent* otherCollider = NewObject<UMesh4DComponent>();
	otherCollider->Mesh4D = mesh;
	otherCollider->EnableCollision = true;
	otherCollider->SetGlobalPosition4D(collider->GetGlobalPosition4D());
	server->RegisterCollider(collider);
	server->RegisterCollider(otherCollider);
	TestTrue(TEXT("Any-hit query succeeds."), server->QueryPointCollision(worldInside, collisions, true));
	TestEqual(TEXT("Any-hit query appends exactly one hit."), collisions.Num(), 1);
	TestTrue(TEXT("All-hit query succeeds."), server->QueryPointCollision(worldInside, collisions));
	TestEqual(TEXT("All-hit query appends both hits without clearing the array."), collisions.Num(), 3);
	server->UnregisterCollider(otherCollider);
	collider->EnableCollision = false;
	collisions.Reset();
	TestFalse(TEXT("Disabling collision takes effect without recreating physics state."), server->QueryPointCollision(worldInside, collisions));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
