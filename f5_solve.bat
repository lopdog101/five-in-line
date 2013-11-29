setlocal
:begin

..\..\wget\wget -O key_file "%url%?src_name=%src_name%&cmd=get_job%own_root%"

@if ERRORLEVEL 1 exit
@for /F "tokens=* delims= " %%i in ('type key_file') do set jb=%%i
@if ERRORLEVEL 1 exit
@for /F "tokens=2,3,* delims= " %%i in ('..\..\solver.exe %jb%') do (
@set n=%%i
@set w=%%j
@set f=%%k
)
@if ERRORLEVEL 1 exit
..\..\wget\wget -O res_file "%url%?src_name=%src_name%&cmd=save_job&key=%jb%&n=%n%&w=%w%&f=%f%"
echo %res%

@if ERRORLEVEL 1 exit

goto begin
endlocal
