#include "Unreal4DRenderingModule.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

DEFINE_LOG_CATEGORY(LogUnreal4DRendering);

void FUnreal4DRenderingModule::StartupModule()
{
	const TSharedPtr<IPlugin> unreal4DPlugin = IPluginManager::Get().FindPlugin(TEXT("Unreal4D"));
	checkf(unreal4DPlugin.IsValid(), TEXT("The Unreal4D plugin descriptor could not be found."));

	const FString unreal4DShaderDirectory = FPaths::Combine(unreal4DPlugin->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Unreal4D"), unreal4DShaderDirectory);
}

IMPLEMENT_MODULE(FUnreal4DRenderingModule, Unreal4DRendering)
