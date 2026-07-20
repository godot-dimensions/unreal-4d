#include "Unreal4DCoreModule.h"

#include "Misc/CoreDelegates.h"
#include "Model/Mesh/Tetra/TetraMesh4D.h"

DEFINE_LOG_CATEGORY(LogUnreal4DCore);

void FUnreal4DCoreModule::StartupModule()
{
	UTetraMesh4D::InitFallbackMaterial();
	EnginePreExitHandle = FCoreDelegates::OnEnginePreExit.AddStatic(&UTetraMesh4D::CleanupFallbackMaterial);
}

void FUnreal4DCoreModule::ShutdownModule()
{
	FCoreDelegates::OnEnginePreExit.Remove(EnginePreExitHandle);
	EnginePreExitHandle.Reset();

	// Normal engine shutdown has already cleaned this UObject up through OnEnginePreExit,
	// before the UObject array is purged. This fallback handles an early module unload.
	if (!IsEngineExitRequested())
	{
		UTetraMesh4D::CleanupFallbackMaterial();
	}
}

IMPLEMENT_MODULE(FUnreal4DCoreModule, Unreal4DCore)
