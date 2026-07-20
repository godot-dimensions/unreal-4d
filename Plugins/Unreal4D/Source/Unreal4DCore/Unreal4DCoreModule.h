#pragma once

#include "Unreal4DDefines.h"

#include "Modules/ModuleManager.h"

class FUnreal4DCoreModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FDelegateHandle EnginePreExitHandle;
};
