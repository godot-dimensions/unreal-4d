/*
 * GPU-side representation of 4D camera state.
 *
 * Defines a render resource that encapsulates a uniform buffer storing 4D camera
 * data (position and basis vectors), along with additional 3D camera orientation
 * data used for rendering.
 *
 * This data is uploaded to the GPU and consumed by the vertex factory shader
 * to perform 4D-to-3D transformations and slicing in the rendering pipeline.
 */

#pragma once

#include "../Unreal4DRenderingDefines.h"

#include "RenderResource.h"
#include "ShaderParameterMacros.h"
#include "Templates/SharedPointer.h"

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(CameraUniform4DShaderParameters, UNREAL4DRENDERING_API)
	SHADER_PARAMETER(FVector4f, Camera4DPosition)

	SHADER_PARAMETER(FMatrix44f, Camera4DRotation)

	SHADER_PARAMETER(FMatrix44f, Camera3DRotation)


	//SHADER_PARAMETER(FVector4f, Camera4DBasisX)
	//SHADER_PARAMETER(FVector4f, Camera4DBasisY)
	//SHADER_PARAMETER(FVector4f, Camera4DBasisZ)
	//SHADER_PARAMETER(FVector4f, Camera4DBasisW)

	//SHADER_PARAMETER(FVector4f, Camera3DBasisX)
	//SHADER_PARAMETER(FVector4f, Camera3DBasisY)
	//SHADER_PARAMETER(FVector4f, Camera3DBasisZ)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

class UNREAL4DRENDERING_API FCameraUniform4DRenderResource : public FRenderResource
{
public:
	virtual void InitRHI(FRHICommandListBase& InRHICmdList) override;
	virtual void ReleaseRHI() override;

	void UpdateUniformData(
		CameraUniform4DShaderParameters InCameraUniform4DShaderParameters,
		FRHICommandList& InRHICmdList);

	FRHIUniformBuffer* GetUniformBuffer() const
	{
		return CameraUniformBufferRef.GetReference();
	}

	TUniformBufferRef<CameraUniform4DShaderParameters> GetUniformBufferRef() const
	{
		return CameraUniformBufferRef;
	}

private:
	TUniformBufferRef<CameraUniform4DShaderParameters> CameraUniformBufferRef;
	CameraUniform4DShaderParameters CameraUniformShaderParametersData;
};

class UNREAL4DRENDERING_API FCamera4DRenderState
{
public:
	FCameraUniform4DRenderResource Resource;
};

/*
 * Thread-safe shared pointer to FCamera4DRenderState.
 *
 * Used to safely share the camera render state between the game thread
 * (URenderingServer4D subsystem) and the render thread (FTetraMesh4DVertexFactory),
 * ensuring correct lifetime management and preventing use-after-free.
 */
using FCamera4DRenderStatePtr = TSharedPtr<FCamera4DRenderState, ESPMode::ThreadSafe>;
