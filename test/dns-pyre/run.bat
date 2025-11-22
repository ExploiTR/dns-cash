@echo off
SET SERVER=127.0.0.1:6073
SET NUM_INSTANCES=16
SET CONCURRENCY=5000
SET DURATION=60s

echo Launching UNLIMITED RATE benchmark...
echo WARNING: This will attempt to saturate your system!
echo.

for /L %%i in (1,1,%NUM_INSTANCES%) do (
    start /B dnspyre.exe --server %SERVER% ^
        --concurrency %CONCURRENCY% ^
        --duration %DURATION% ^
        --query-per-conn 0 ^
        test.example.com
)

echo All instances running at MAXIMUM rate!
pause
