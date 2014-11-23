setlocal
set url=http://fiveinline.info/solutions/root/solve.php
set src_name=somebody
set stored_deep=2
set lookup_deep=3
set process_count=%NUMBER_OF_PROCESSORS%
set own_root=

@for /L %%i in (1,1,%process_count%) do (
@mkdir spawn\%%i
cd spawn\%%i
start ..\..\f5_solve.bat
cd ..\..
)

endlocal
