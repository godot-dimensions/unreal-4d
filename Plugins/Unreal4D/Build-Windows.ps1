[CmdletBinding()]
param(
	[string]$EngineRoot,
	[string]$PackageDir,
	[string]$TargetPlatforms = "Win64"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-EngineRoot
{
	param([string]$RequestedRoot)

	if ([string]::IsNullOrWhiteSpace($RequestedRoot))
	{
		$RequestedRoot = $env:UNREAL_ENGINE_ROOT
	}

	if ([string]::IsNullOrWhiteSpace($RequestedRoot))
	{
		$RequestedRoot = Join-Path $PSScriptRoot "..\..\..\UnrealEngine6"
	}

	if (-not (Test-Path -LiteralPath $RequestedRoot -PathType Container))
	{
		throw "Unreal Engine was not found at '$RequestedRoot'. Pass -EngineRoot or set UNREAL_ENGINE_ROOT."
	}

	return (Resolve-Path -LiteralPath $RequestedRoot).Path
}

function Get-AbsolutePath
{
	param([string]$Path)

	if ([IO.Path]::IsPathRooted($Path))
	{
		return [IO.Path]::GetFullPath($Path)
	}

	return [IO.Path]::GetFullPath((Join-Path (Get-Location).Path $Path))
}

# The usage of "$PSScriptRoot" ensures that this works regardless of the current working directory of whatever calls this script.
$ResolvedEngineRoot = Resolve-EngineRoot $EngineRoot
#$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..")).Path
$PluginFile = Join-Path $PSScriptRoot "Unreal4D.uplugin"
$RunUAT = Join-Path $ResolvedEngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

if ([string]::IsNullOrWhiteSpace($PackageDir))
{
	$PackageDir = Join-Path $PSScriptRoot "BuildArtifacts\Unreal4D-$TargetPlatforms"
}
else
{
	$PackageDir = Get-AbsolutePath $PackageDir
}

if (-not (Test-Path -LiteralPath $RunUAT -PathType Leaf))
{
	throw "RunUAT.bat was not found beneath '$ResolvedEngineRoot'."
}

& $RunUAT BuildPlugin "-Plugin=$PluginFile" "-Package=$PackageDir" "-TargetPlatforms=$TargetPlatforms" "-NoXGE"
if ($LASTEXITCODE -ne 0)
{
	throw "BuildPlugin failed with exit code $LASTEXITCODE."
}

Write-Host "Successfully packaged Unreal4D to '$PackageDir'."
