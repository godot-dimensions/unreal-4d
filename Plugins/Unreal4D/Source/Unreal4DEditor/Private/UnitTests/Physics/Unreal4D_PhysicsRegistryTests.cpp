#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../../../../Unreal4DCore/Physics/Bodies/Area4D.h"
#include "../../../../Unreal4DCore/Physics/Bodies/StaticBody4D.h"
#include "../../../../Unreal4DCore/Physics/Server/PhysicsServer4D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_PhysicsRegistrySnapshots, "Unreal4D.Physics.RegistrySnapshots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_PhysicsRegistrySnapshots::RunTest(const FString& Parameters)
{
	UPhysicsServer4DWorldSubsystem* server = NewObject<UPhysicsServer4DWorldSubsystem>();
	UArea4DComponent* area = NewObject<UArea4DComponent>();
	UStaticBody4DComponent* body = NewObject<UStaticBody4DComponent>();
	server->RegisterArea(area);
	server->RegisterPhysicsBody(body);
	const TArray<TWeakObjectPtr<UArea4DComponent>> areas = server->GetAreaComponents();
	const TArray<TWeakObjectPtr<UPhysicsBody4DComponent>> bodies = server->GetPhysicsBodyComponents();
	if (!TestEqual(TEXT("Area snapshot contains the registered area."), areas.Num(), 1)
		|| !TestEqual(TEXT("Body snapshot contains the registered body."), bodies.Num(), 1))
	{
		return false;
	}
	TestTrue(TEXT("Area snapshot resolves the registered component."), areas[0].Get() == area);
	TestTrue(TEXT("Body snapshot resolves the registered component."), bodies[0].Get() == body);
	server->UnregisterArea(area);
	server->UnregisterPhysicsBody(body);
	TestEqual(TEXT("Area registry reflects unregistration."), server->GetAreaComponents().Num(), 0);
	TestEqual(TEXT("Body registry reflects unregistration."), server->GetPhysicsBodyComponents().Num(), 0);
	TestTrue(TEXT("Unregistration leaves the area snapshot intact."), areas.Num() == 1 && areas[0].Get() == area);
	TestTrue(TEXT("Unregistration leaves the body snapshot intact."), bodies.Num() == 1 && bodies[0].Get() == body);
	area->DestroyComponent();
	body->DestroyComponent();
	TestTrue(TEXT("Destroyed area resolves to null in the existing snapshot."), areas[0].Get() == nullptr);
	TestTrue(TEXT("Destroyed body resolves to null in the existing snapshot."), bodies[0].Get() == nullptr);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
