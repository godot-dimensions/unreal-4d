@echo off
setlocal

rem The "%~dp0" resolves to the current drive and path of this batch file.
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Run-Windows.ps1" %*
set "RUN_EXIT_CODE=%ERRORLEVEL%"

echo.
if not "%RUN_EXIT_CODE%"=="0" echo The Unreal4D example could not be built or opened. Review the error above.
pause

exit /b %RUN_EXIT_CODE%
