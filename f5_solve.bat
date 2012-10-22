setlocal
:begin

..\..\wget\wget -O key_file "http://f5.vnetgis.com/solve.php?cmd=get_job"
@if ERRORLEVEL 1 exit
@for /F "tokens=* delims= " %%i in ('type key_file') do set jb=%%i
@if ERRORLEVEL 1 exit
@for /F "tokens=2,3,* delims= " %%i in ('..\..\solver.exe %jb%') do (
@set n=%%i
@set w=%%j
@set f=%%k
)
@if ERRORLEVEL 1 exit
..\..\wget\wget -O res_file "http://f5.vnetgis.com/solve.php?cmd=save_job&key=%jb%&n=%n%&w=%w%&f=%f%"
echo %res%

@if ERRORLEVEL 1 exit

goto begin
endlocal
