@echo off
setlocal enabledelayedexpansion

echo Starting Portfolio Optimizer Runner
echo Running new instances every 2.5 seconds
echo Press Ctrl+C to stop

:: Check if Python is installed
where python >nul 2>nul
if %errorlevel% neq 0 (
    echo Python is not installed or not in PATH
    echo Please install Python and ensure it's in your PATH
    pause
    exit /b 1
)

:: Check if the Python script exists
if not exist "run_optimizer.py" (
    echo run_optimizer.py not found in current directory
    echo Current directory: %CD%
    pause
    exit /b 1
)

:: Show Python version
python --version

set "logfile=optimizer_log.txt"
echo [%date% %time%] Starting optimizer >> "%logfile%"
echo [%date% %time%] Python version: >> "%logfile%"
python --version >> "%logfile%" 2>&1

:loop
echo [%date% %time%] Starting new instance >> "%logfile%"
echo Running Python script...
python run_optimizer.py >> "%logfile%" 2>&1

if %errorlevel% neq 0 (
    echo [%date% %time%] Error occurred, retrying in 2.5 seconds >> "%logfile%"
    echo Error occurred, check optimizer_log.txt for details
    timeout /t 2 /nobreak > nul
    goto loop
)

echo Instance completed successfully
timeout /t 2 /nobreak > nul
goto loop 