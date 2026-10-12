#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../../../../Unreal4DCore/Physics/Server/PhysicsServer4D.h"
#include "../../../../Unreal4DCore/Scene/Components/Mesh4DComponent.h"
#include "../../../../Unreal4DCore/Model/Mesh/Tetra/BoxTetraMesh4D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_PointCollision, "Unreal4D.Physics.PointCollision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_PointCollision::RunTest(const FString& Parameters)
{
	UMesh4DComponent* meshComp = NewObject<UMesh4DComponent>();
	UBoxTetraMesh4D* mesh = NewObject<UBoxTetraMesh4D>(meshComp);
	mesh->SetSize(FVector4(2.0, 2.0, 2.0, 2.0));
	meshComp->Mesh4D = mesh;
	FCollisionManifold4D collision;
	const FVector4 insidePoint(0.75, 0.0, 0.0, 0.0);
	TestFalse(TEXT("Collision is disabled by default."), meshComp->QueryPointCollision(insidePoint, collision));
	meshComp->EnableCollision = true;
	TestTrue(TEXT("Interior point hits a plain Unreal4D mesh without Chaos state."), meshComp->QueryPointCollision(insidePoint, collision));
	TestTrue(TEXT("Hit references the original mesh component."), collision.ColliderPrimitive == meshComp && collision.IsValid());
	TestTrue(TEXT("Surface point is on the nearest box face."), collision.Point.Equals(FVector4(1.0, 0.0, 0.0, 0.0), 1e-6));
	TestTrue(TEXT("Normal points out of the mesh."), collision.Normal.Equals(FVector4(1.0, 0.0, 0.0, 0.0), 1e-6));
	TestTrue(TEXT("Signed penetration is preserved."), FMath::IsNearlyEqual(collision.Penetration, -0.25f));
	TestFalse(TEXT("Exterior XYZ point misses."), meshComp->QueryPointCollision(FVector4(2.0, 0.0, 0.0, 0.0), collision));
	TestFalse(TEXT("Exterior W point misses."), meshComp->QueryPointCollision(FVector4(0.0, 0.0, 0.0, 2.0), collision));
	TestTrue(TEXT("Boundary point hits."), meshComp->QueryPointCollision(FVector4(1.0, 0.0, 0.0, 0.0), collision));
	meshComp->SetGlobalPosition4D(FVector4(10.0, 20.0, 30.0, 40.0));
	const FVector4 worldInside = meshComp->GetGlobalTransform4D().TransformPosition(insidePoint);
	TestTrue(TEXT("Query uses all four world coordinates."), meshComp->QueryPointCollision(worldInside, collision));
	TestTrue(TEXT("Surface point is returned in world space."), collision.Point.Equals(FVector4(11.0, 20.0, 30.0, 40.0), 1e-6));
	TestFalse(TEXT("An untranslated point misses a translated mesh."), meshComp->QueryPointCollision(insidePoint, collision));
	UBoxTetraMesh4D* smallerMesh = NewObject<UBoxTetraMesh4D>(meshComp);
	smallerMesh->SetSize(FVector4(1.0, 1.0, 1.0, 1.0));
	meshComp->CollisionMesh4D = smallerMesh;
	TestFalse(TEXT("Collision mesh override replaces the rendered mesh for queries."), meshComp->QueryPointCollision(worldInside, collision));
	meshComp->CollisionMesh4D = nullptr;
	UPhysicsServer4DWorldSubsystem* server = NewObject<UPhysicsServer4DWorldSubsystem>();
	server->RegisterPrimitive(meshComp);
	server->RegisterPrimitive(meshComp);
	TArray<FCollisionManifold4D> collisions;
	TestTrue(TEXT("Server dispatches registered meshComp queries."), server->QueryPointCollision(worldInside, collisions));
	TestEqual(TEXT("Duplicate registration produces one hit."), collisions.Num(), 1);
	server->UnregisterPrimitive(meshComp);
	collisions.Reset();
	TestFalse(TEXT("Unregistered colliders are excluded."), server->QueryPointCollision(worldInside, collisions));
	UMesh4DComponent* otherMeshComp = NewObject<UMesh4DComponent>();
	otherMeshComp->Mesh4D = mesh;
	otherMeshComp->EnableCollision = true;
	otherMeshComp->SetGlobalPosition4D(meshComp->GetGlobalPosition4D());
	server->RegisterPrimitive(meshComp);
	server->RegisterPrimitive(otherMeshComp);
	TestTrue(TEXT("Any-hit query succeeds."), server->QueryPointCollision(worldInside, collisions, true));
	TestEqual(TEXT("Any-hit query appends exactly one hit."), collisions.Num(), 1);
	TestTrue(TEXT("All-hit query succeeds."), server->QueryPointCollision(worldInside, collisions));
	TestEqual(TEXT("All-hit query appends both hits without clearing the array."), collisions.Num(), 3);
	server->UnregisterPrimitive(otherMeshComp);
	meshComp->EnableCollision = false;
	collisions.Reset();
	TestFalse(TEXT("Disabling collision takes effect without recreating physics state."), server->QueryPointCollision(worldInside, collisions));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
