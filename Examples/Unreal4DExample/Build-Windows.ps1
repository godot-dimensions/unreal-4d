[CmdletBinding()]
param(
	[string]$EngineRoot,

	[ValidateSet("Editor", "Game")]
	[string]$Target = "Editor",

	[ValidateSet("Development", "DebugGame", "Shipping")]
	[string]$Configuration = "Development",

	[switch]$SkipProjectFiles,

	[switch]$GenerateOnly
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

function Invoke-Checked
{
	param(
		[string]$Executable,
		[string[]]$Arguments
	)

	& $Executable @Arguments
	if ($LASTEXITCODE -ne 0)
	{
		throw "'$Executable' failed with exit code $LASTEXITCODE."
	}
}

function Write-JsonFile
{
	param(
		[string]$Path,
		[object]$Value
	)

	$Json = $Value | ConvertTo-Json -Depth 100
	$Utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
	[System.IO.File]::WriteAllText($Path, $Json + [Environment]::NewLine, $Utf8WithoutBom)
}

function Write-RepositoryVSCodeConfiguration
{
	param(
		[string]$RepositoryRoot,
		[string]$ExampleProjectDirectory
	)

	$GeneratedVSCodeDirectory = Join-Path $ExampleProjectDirectory ".vscode"
	$GeneratedCppConfiguration = Join-Path $GeneratedVSCodeDirectory "c_cpp_properties.json"
	$GeneratedWorkspace = Join-Path $ExampleProjectDirectory "Unreal4DExample.code-workspace"

	if (-not (Test-Path -LiteralPath $GeneratedCppConfiguration -PathType Leaf))
	{
		throw "UnrealBuildTool did not generate '$GeneratedCppConfiguration'."
	}

	if (-not (Test-Path -LiteralPath $GeneratedWorkspace -PathType Leaf))
	{
		throw "UnrealBuildTool did not generate '$GeneratedWorkspace'."
	}

	$RepositoryVSCodeDirectory = Join-Path $RepositoryRoot ".vscode"
	New-Item -ItemType Directory -Path $RepositoryVSCodeDirectory -Force | Out-Null
	Copy-Item -LiteralPath $GeneratedCppConfiguration -Destination (Join-Path $RepositoryVSCodeDirectory "c_cpp_properties.json") -Force

	$CppConfiguration = Get-Content -LiteralPath $GeneratedCppConfiguration -Raw | ConvertFrom-Json
	$CppConfigurationWithCompileCommands = @($CppConfiguration.configurations | Where-Object {
		-not [string]::IsNullOrWhiteSpace($_.compileCommands)
	} | Select-Object -First 1)
	if ($CppConfigurationWithCompileCommands.Count -eq 0)
	{
		throw "UnrealBuildTool did not provide a compile database in '$GeneratedCppConfiguration'."
	}

	$WorkspaceCompileCommands = $CppConfigurationWithCompileCommands[0].compileCommands
	if (-not (Test-Path -LiteralPath $WorkspaceCompileCommands -PathType Leaf))
	{
		throw "UnrealBuildTool's compile database was not found at '$WorkspaceCompileCommands'."
	}

	$Workspace = Get-Content -LiteralPath $GeneratedWorkspace -Raw | ConvertFrom-Json
	$PluginWorkspacePath = "../../Plugins/Unreal4D"
	$WorkspaceFoldersWithoutPlugin = @($Workspace.folders | Where-Object {
		$_.name -ne "Unreal4D" -and $_.path -ne $PluginWorkspacePath
	})
	$PluginWorkspaceFolder = [pscustomobject][ordered]@{
		name = "Unreal4D"
		path = $PluginWorkspacePath
	}
	$Workspace.folders = @($WorkspaceFoldersWithoutPlugin | Select-Object -First 1)
	$Workspace.folders += $PluginWorkspaceFolder
	$Workspace.folders += @($WorkspaceFoldersWithoutPlugin | Select-Object -Skip 1)
	$Workspace.settings | Add-Member -NotePropertyName "C_Cpp.default.compileCommands" -NotePropertyValue $WorkspaceCompileCommands -Force
	$Workspace.settings | Add-Member -NotePropertyName "dotnet.defaultSolution" -NotePropertyValue (Join-Path $ExampleProjectDirectory "Unreal4DExample.sln") -Force
	Write-JsonFile $GeneratedWorkspace $Workspace

	$Settings = [ordered]@{}
	foreach ($Property in $Workspace.settings.PSObject.Properties)
	{
		$Settings[$Property.Name] = $Property.Value
	}
	$Settings["dotnet.defaultSolution"] = "Examples/Unreal4DExample/Unreal4DExample.sln"

	$Recommendations = @($Workspace.extensions.recommendations)
	$Recommendations += "ms-dotnettools.csdevkit"
	$Recommendations += "ms-vscode.powershell"
	$Extensions = [ordered]@{
		recommendations = @($Recommendations | Select-Object -Unique)
	}

	Write-JsonFile (Join-Path $RepositoryVSCodeDirectory "settings.json") $Settings
	Write-JsonFile (Join-Path $RepositoryVSCodeDirectory "extensions.json") $Extensions
	Write-JsonFile (Join-Path $RepositoryVSCodeDirectory "tasks.json") $Workspace.tasks
	Write-JsonFile (Join-Path $RepositoryVSCodeDirectory "launch.json") $Workspace.launch

	Write-Host "Added the Unreal4D plugin root and IntelliSense settings to '$GeneratedWorkspace'."
	Write-Host "Generated repository-local VS Code configuration in '$RepositoryVSCodeDirectory'."
}

function Set-VisualStudioPluginFilterAlias
{
	param([string]$ExampleProjectDirectory)

	$FiltersFile = Join-Path $ExampleProjectDirectory "Intermediate\ProjectFiles\Unreal4DExample.vcxproj.filters"
	if (-not (Test-Path -LiteralPath $FiltersFile -PathType Leaf))
	{
		throw "UnrealBuildTool did not generate '$FiltersFile'."
	}

	# UnrealBuildTool places an external plugin beneath filters named "..\..\Plugins\Unreal4D".
	# Alias only Visual Studio's display filters; do not alter the physical Include paths.
	$ExternalFilterRoot = "..\..\Plugins\Unreal4D"
	$PluginFilterRoot = "Unreal4D"
	$Content = [System.IO.File]::ReadAllText($FiltersFile)
	$AliasedContent = $Content.Replace(
		"<Filter Include=`"$ExternalFilterRoot",
		"<Filter Include=`"$PluginFilterRoot"
	).Replace(
		"<Filter>$ExternalFilterRoot",
		"<Filter>$PluginFilterRoot"
	)

	if ($AliasedContent -eq $Content)
	{
		Write-Warning "The expected external Unreal4D Visual Studio filters were not found in '$FiltersFile'."
		return
	}

	$Utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
	[System.IO.File]::WriteAllText($FiltersFile, $AliasedContent, $Utf8WithoutBom)
	Write-Host "Aliased the external plugin to the top-level 'Unreal4D' filter in the example Visual Studio project."
}

# The usage of "$PSScriptRoot" ensures that this works regardless of the current working directory of whatever calls this script.
$ResolvedEngineRoot = Resolve-EngineRoot $EngineRoot
$ProjectFile = Join-Path $PSScriptRoot "Unreal4DExample.uproject"
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..")).Path
$GenerateProjectFiles = Join-Path $ResolvedEngineRoot "GenerateProjectFiles.bat"
$BuildTool = Join-Path $ResolvedEngineRoot "Engine\Build\BatchFiles\Build.bat"

if ($SkipProjectFiles -and $GenerateOnly)
{
	throw "-SkipProjectFiles and -GenerateOnly cannot be used together."
}

if (-not $SkipProjectFiles)
{
	if (-not (Test-Path -LiteralPath $GenerateProjectFiles -PathType Leaf))
	{
		throw "GenerateProjectFiles.bat was not found beneath '$ResolvedEngineRoot'."
	}

	Invoke-Checked $GenerateProjectFiles @(
		"-Project=$ProjectFile",
		"-Game",
		"-Engine",
		"-CurrentPlatform",
		"-Automated",
		"-VisualStudio",
		"-VSCode",
		"-NoXGE"
	)

	Set-VisualStudioPluginFilterAlias $PSScriptRoot
	Write-RepositoryVSCodeConfiguration $RepositoryRoot $PSScriptRoot
}

if ($GenerateOnly)
{
	Write-Host "Successfully generated Visual Studio and VS Code project metadata."
	return
}

if (-not (Test-Path -LiteralPath $BuildTool -PathType Leaf))
{
	throw "Build.bat was not found beneath '$ResolvedEngineRoot'."
}

$TargetName = if ($Target -eq "Editor") { "Unreal4DExampleEditor" } else { "Unreal4DExample" }

if ($Target -eq "Editor")
{
	# This separate program must match the engine's shader protocol after switching branches.
	Invoke-Checked $BuildTool @(
		"ShaderCompileWorker",
		"Win64",
		"Development",
		"-WaitMutex",
		"-NoXGE"
	)
}

Invoke-Checked $BuildTool @(
	$TargetName,
	"Win64",
	$Configuration,
	"-Project=$ProjectFile",
	"-WaitMutex",
	"-NoXGE"
)

Write-Host "Successfully built $TargetName for Win64 $Configuration."
