@echo off
setlocal

rem High-level convenience wrapper. Forwarded arguments must be valid for both PowerShell scripts.
set "BUILD_EXIT_CODE=0"

echo ===============================================
echo Unreal4D Windows Build
echo ===============================================

echo.
echo Step 1 of 2: Packaging the Unreal4D plugin...
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Plugins\Unreal4D\Build-Windows.ps1" %*
set "BUILD_EXIT_CODE=%ERRORLEVEL%"
if not "%BUILD_EXIT_CODE%"=="0" goto finish

echo.
echo Step 2 of 2: Setting up IDEs and building the example...
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Examples\Unreal4DExample\Build-Windows.ps1" %*
set "BUILD_EXIT_CODE=%ERRORLEVEL%"

:finish
echo.
if not "%BUILD_EXIT_CODE%"=="0" (
	echo Build failed with exit code %BUILD_EXIT_CODE%.
) else (
	echo Unreal4D plugin and example are ready.
)
pause

exit /b %BUILD_EXIT_CODE%
