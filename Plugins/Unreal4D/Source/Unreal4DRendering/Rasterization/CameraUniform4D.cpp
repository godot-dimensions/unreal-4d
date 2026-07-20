#include "CameraUniform4D.h"

#include "RHI.h"

void FCameraUniform4DRenderResource::InitRHI(FRHICommandListBase& InRHICmdList)
{
	CameraUniformShaderParametersData.Camera4DPosition = FVector4f(0.0f, 0.0f, 0.0f, 0.0f);
	CameraUniformShaderParametersData.Camera4DRotation = FMatrix44f::Identity;
	CameraUniformShaderParametersData.Camera3DRotation = FMatrix44f::Identity;

	CameraUniformBufferRef = TUniformBufferRef<CameraUniform4DShaderParameters>::CreateUniformBufferImmediate(
		CameraUniformShaderParametersData,
		UniformBuffer_MultiFrame);
}

void FCameraUniform4DRenderResource::ReleaseRHI()
{
	CameraUniformBufferRef.SafeRelease();
}

void FCameraUniform4DRenderResource::UpdateUniformData(
	CameraUniform4DShaderParameters InCameraUniform4DShaderParameters,
	FRHICommandList& InRHICmdList)
{
	CameraUniformShaderParametersData = InCameraUniform4DShaderParameters;

	ensure(CameraUniformBufferRef.IsValid());
	if (CameraUniformBufferRef.IsValid())
	{
		CameraUniformBufferRef.UpdateUniformBufferImmediate(InRHICmdList, CameraUniformShaderParametersData);
	}
}

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(CameraUniform4DShaderParameters, "Camera4D");
