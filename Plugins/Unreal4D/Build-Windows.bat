@echo off
setlocal

rem The "%~dp0" resolves to the current drive and path of this batch file.
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Build-Windows.ps1" %*
set "BUILD_EXIT_CODE=%ERRORLEVEL%"

echo.
if not "%BUILD_EXIT_CODE%"=="0" echo Build failed with exit code %BUILD_EXIT_CODE%.
pause

exit /b %BUILD_EXIT_CODE%
