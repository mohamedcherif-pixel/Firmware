@echo off
REM GitHub CLI Authentication Wrapper

echo Attempting to authenticate with GitHub...
echo.

REM Try direct command first
gh auth login

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS! GitHub CLI authenticated.
    goto end
)

REM If that fails, try full path
if exist "C:\Program Files\GitHub CLI\bin\gh.exe" (
    echo Trying full path...
    "C:\Program Files\GitHub CLI\bin\gh.exe" auth login
    goto end
)

echo.
echo ERROR: GitHub CLI not found
echo Please restart PowerShell and try: gh auth login
echo If that doesn't work, reinstall: winget install GitHub.cli --force

:end
echo.
pause
