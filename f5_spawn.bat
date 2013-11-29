setlocal
set url=http://fiveinline.info/solutions/root/solve.php
set src_name=somebody
rem set stored_deep=2
rem set lookup_deep=5
set process_count=%NUMBER_OF_PROCESSORS%
set own_root=
rem set own_root=^&root=ff0002ff0101000001010101020202

@for /L %%i in (1,1,%process_count%) do (
@mkdir spawn\%%i
cd spawn\%%i
start ..\..\f5_solve.bat
cd ..\..
)

endlocal
