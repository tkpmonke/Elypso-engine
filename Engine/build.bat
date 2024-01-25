:: Copyright (C) 2024 Greenlaser
:: This program comes with ABSOLUTELY NO WARRANTY.
:: This is free software, and you are welcome to redistribute it under certain conditions.
:: Read LICENSE.md and EULA.md for more information.

@echo off
:: Batch script to build the executable and create the installer for the engine

:: Reusable message types printed to console
set "enexc=[ENGINE_EXCEPTION]"
set "eninf=[ENGINE_INFO]"
set "encln=[ENGINE_CLEANUP]"
set "cminf=[CMAKE_INFO]"
set "cmerr=[CMAKE_EXCEPTION]"
set "cmsuc=[CMAKE_SUCCESS]"
set "cpinf=[CPACK_INFO]"
set "cperr=[CPACK_EXCEPTION]"
set "cpsuc=[CPACK_SUCCESS]"

set "documentsPath=%USERPROFILE%\Documents\Elypso engine"
set "outPath=%~dp0out"
set "vsPath=%~dp0.vs"

set "buildPath=%~dp0build"

:: Can not run build.bat without administrative privileges
NET SESSION >nul 2>&1
if %errorlevel% neq 0 (
	echo %enexc% This script requires administrative privileges. Please run as administrator.
	pause
	exit /b 1
)

:menu
cls

echo Elypso engine setup
echo.
echo Copyright (C) 2024 Greenlaser
echo.
echo This program comes with ABSOLUTELY NO WARRANTY.
echo This is free software, and you are welcome to redistribute it under certain conditions.
echo Read LICENSE.md and EULA.md for more information.

echo.

echo ========================================================

echo.

echo Write the number of your choice to choose the action.
echo.
echo 1. Reconfigure CMake
echo 2. Build Elypso engine
echo 3. Exit
echo.
echo 9. Clean Visual Studio (DELETES OUT AND .VS FOLDERS)
echo 0. Clean engine (DELETES BUILD AND ENGINE DOCUMENTS FOLDERS)
echo.
set /p choice="Choice: "

:: Process user input
if "%choice%"=="1" goto cmake
if "%choice%"=="2" goto build
if "%choice%"=="3" exit

if "%choice%"=="9" goto cleanvs
if "%choice%"=="0" goto cleanen

echo %prexc% Invalid choice! Please enter a valid number.
pause
goto menu

:cmake
:: Change to the script directory
cd /d "%~dp0"
	
:: Clean the build directory before configuration
if exist "%buildPath%" (
	echo %encln% Deleted folder: build
	rd /s /q "%buildPath%"
)
mkdir "%buildPath%"
cd "%buildPath%"

echo %cminf% Started CMake configuration.

:: Configure the project (Release build)
cmake -DCMAKE_BUILD_TYPE=Release ..

if %errorlevel% neq 0 (
	echo %cmerr% CMake configuration failed.
) else (
	echo %cmsuc% Cmake configuration succeeded!
)
	
pause
goto menu

:build
:: Change to the script directory
cd /d "%~dp0"
	
if not exist "%buildPath%" (
	echo Did not find build folder. Please run 'Reconfigure CMake' before building.
) else (
	cd "%buildPath%"
		
	:: Build the project
	echo %cminf% Started build generation.
	cmake --build . --config Release
	
	if %errorlevel% neq 0 (
		echo %cmerr% Build failed because Elypso_engine.exe did not get generated properly.
	) else (
		echo %cmsuc% Build succeeded!
		goto iconchange
	)
)

pause
goto menu

:iconchange

cd /d "%~dp0"

set "currentdir=%CD%"
set "runpath=%currentdir%\..\IconChanger\build\Release\IconChanger.exe"
set "exepath=%currentdir%\build\Release\Elypso_engine.exe"
set "iconpath=%currentdir%\files\icon.ico"

if not exist "%runpath%" (
	echo %enexc% Did not find %runpath%! Either you did not build IconChange.exe or it is not in the right location!
)

:: Look for IconChanger.exe
if not exist "%runpath%" (
	echo %enexc% Did not find "%runpath%". Either you did not build IconChange.exe or it is not in the right location!
	
	pause
	exit /b 1
) else (
	echo Found IconChanger.exe at "%runpath%".
	echo.
)

:: Executable checks
if "%exepath%" == "" (
	echo Error: Exe path is empty!
	
	pause
	exit /b 1
)
if not exist "%exepath%" (
	echo Error: "%exepath%" does not exist! Please insert a valid path to the executable!
	
	pause
	exit /b 1
)
for %%F in ("%exepath%") do set "exetype=%%~xF"
if /I "%exetype%" NEQ ".exe" (
	echo Error: Extension type for "%exepath%" was not valid! It must be '.exe'.
) else (
	echo Found executable at "%exepath%".
	echo.
)

:: Icon checks
if "%iconpath%" == "" (
	echo Error: Icon path is empty!
	
	pause
	exit /b 1
)
if not exist "%iconpath%" (
	echo Error: "%iconpath%" does not exist! Please insert a valid path to the icon!
	
	pause
	exit /b 1
)
for %%F in ("%iconpath%") do set "icontype=%%~xF"
if /I "%icontype%" NEQ ".ico" (
	echo Error: Extension type for "%iconpath%" was not valid! It must be '.ico'.
	
	pause
	exit /b 1
) else (
	echo Found icon at "%iconpath%".
	echo.
)

:: Run IconChanger.exe to change Elypso engine icon
"%runpath%" "%exepath%" "%iconpath%"

pause
goto menu

:cleanvs
:: Change to the script directory
cd /d "%~dp0"
	
echo %eninf% Running vs clean...
if not exist "%vsPath%" (
	if not exist "%outPath%" (
		echo %encln% There are no Visual Studio folders to remove.
		pause
		goto menu
	)
)

if exist "%vsPath%" (
	echo %encln% Deleted folder: .vs
	rd /s /q "%vsPath%"
)
echo "%outPath%"
if exist "%outPath%" (
	echo %encln% Deleted folder: out
	rd /s /q "%outPath%"
)
	
pause
goto menu

:cleanen
:: Change to the script directory
cd /d "%~dp0"
	
if not exist "%buildPath%" (
	if not exist "%documentsPath%" (
		echo %encln% There are no engine folders to remove.
		pause
		goto menu
	)
)

if exist "%buildPath%" (
	echo %encln% Deleted folder: build
	rd /s /q "%buildPath%"
)
if exist "%documentsPath%" (
	echo %encln% Deleted folder: Documents/Elypso engine
	rd /s /q "%documentsPath%"
)
	
pause
goto menu