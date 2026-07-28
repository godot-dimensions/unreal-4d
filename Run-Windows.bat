@echo off
setlocal

rem Forward to the example project's Windows launcher.
call "%~dp0Examples\Unreal4DExample\Run-Windows.bat" %*
exit /b %ERRORLEVEL%
