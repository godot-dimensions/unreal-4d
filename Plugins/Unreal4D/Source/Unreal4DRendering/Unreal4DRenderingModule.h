#pragma once

#include "Unreal4DRenderingDefines.h"

#include "Modules/ModuleManager.h"

class FUnreal4DRenderingModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
};
