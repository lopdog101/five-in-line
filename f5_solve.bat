setlocal
:begin

..\..\wget\wget -O key_file "%url%?src_name=%src_name%&cmd=get_job%own_root%"

@if ERRORLEVEL 1 exit
@for /F "tokens=* delims= " %%i in ('type key_file') do set jb=%%i
@if ERRORLEVEL 1 exit

..\..\solver.exe  %jb% >solve_content
@if ERRORLEVEL 1 exit

..\..\wget\wget -O res_file --post-file=solve_content "%url%?src_name=%src_name%&cmd=save_job&key=%jb%&sd=%stored_deep%&ld=%lookup_deep%&ac=%ant_count%&pc=%process_count%"
@if ERRORLEVEL 1 exit

goto begin
endlocal
