setlocal
:begin

for /F "tokens=* delims= " %%i in ('db.exe C:\1\1 get_job') do set jb=%%i
if ERRORLEVEL 1 exit
for /F "tokens=* delims= " %%i in ('solver.exe %jb%') do set res=%%i
if ERRORLEVEL 1 exit
db.exe C:\1\1 save_job %res%
if ERRORLEVEL 1 exit

goto begin
endlocal
