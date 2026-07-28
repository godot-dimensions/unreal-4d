[CmdletBinding()]
param(
	[string]$EngineRoot,

	[ValidateSet("Editor", "Game")]
	[string]$Target = "Editor",

	[ValidateSet("Development", "DebugGame", "Shipping")]
	[string]$Configuration = "Development",

	[switch]$SkipProjectFiles,

	[switch]$GenerateOnly,

	[switch]$SkipBuild
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
		$RequestedRoot = Join-Path $PSScriptRoot "..\..\..\UnrealEngine5"
	}

	if (-not (Test-Path -LiteralPath $RequestedRoot -PathType Container))
	{
		throw "Unreal Engine was not found at '$RequestedRoot'. Pass -EngineRoot or set UNREAL_ENGINE_ROOT."
	}

	return (Resolve-Path -LiteralPath $RequestedRoot).Path
}

$ResolvedEngineRoot = Resolve-EngineRoot $EngineRoot
$ProjectFile = Join-Path $PSScriptRoot "Unreal4DExample.uproject"
$BuildScript = Join-Path $PSScriptRoot "Build-Windows.ps1"
$Editor = Join-Path $ResolvedEngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"

if (-not (Test-Path -LiteralPath $ProjectFile -PathType Leaf))
{
	throw "Project descriptor not found: $ProjectFile"
}

if (-not $SkipBuild)
{
	if (-not (Test-Path -LiteralPath $BuildScript -PathType Leaf))
	{
		throw "Windows build script not found: $BuildScript"
	}

	$BuildParameters = @{}
	foreach ($Parameter in $PSBoundParameters.GetEnumerator())
	{
		if ($Parameter.Key -ne "SkipBuild")
		{
			$BuildParameters[$Parameter.Key] = $Parameter.Value
		}
	}

	& $BuildScript @BuildParameters
}

if (-not (Test-Path -LiteralPath $Editor -PathType Leaf))
{
	throw "Unreal Editor was not found: $Editor"
}

Write-Host "Opening Unreal4DExample in the Unreal Editor..."
& $Editor $ProjectFile
if ($LASTEXITCODE -ne 0)
{
	throw "Unreal Editor exited with code $LASTEXITCODE."
}
