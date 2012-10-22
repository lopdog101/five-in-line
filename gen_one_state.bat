db.exe c:\1\1 init_state "(0,0:X);(1,0:O);(0,1:X)"
db.exe c:\1\1 solve_level

@for /L %%i in (1,1,33) do (
db.exe c:\1\1 get_job
)

db.exe c:\1\1 save_job 00000100010101000200ff02 FF01 empty empty
db.exe c:\1\1 save_job 000001000101000202010002 FF00 empty empty
db.exe c:\1\1 save_job 00000100010101000200FE02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002ff0102 FF00 empty empty
db.exe c:\1\1 save_job 00000100010101000201ff02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002010102 0002 empty empty
db.exe c:\1\1 save_job 00000100010101000200fd02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002000302 FF00 empty empty
db.exe c:\1\1 save_job 000001000101010002ff0002 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002fe0002 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002ffff02 00FF empty empty
db.exe c:\1\1 save_job 000001000101010002020002 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002fe0102 FF00 empty empty
db.exe c:\1\1 save_job 000001000101010002fffe02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002ff0202 FF00 empty empty
db.exe c:\1\1 save_job 00000100010101000201fe02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002010202 0101 empty empty
db.exe c:\1\1 save_job 00000100010101000202ff02 00FF empty empty
db.exe c:\1\1 save_job 000001000101010002020102 00FF empty empty
db.exe c:\1\1 save_job 000001000101010002030002 FF01 empty empty
db.exe c:\1\1 save_job 000001000101010002fefe02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002fe0202 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002ff0302 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002010302 0101 empty empty
db.exe c:\1\1 save_job 00000100010101000202fe02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002020202 FF00 empty empty
db.exe c:\1\1 save_job 00000100010101000203ff02 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002030102 FF01 empty empty
db.exe c:\1\1 save_job 000001000101010002fe0302 0101 empty empty
db.exe c:\1\1 save_job 000001000101010002020302 FF01 empty empty
db.exe c:\1\1 save_job 00000100010101000203fe02 FF01 empty empty
db.exe c:\1\1 save_job 000001000101010002030202 FF01 empty empty
db.exe c:\1\1 save_job 000001000101010002feff02 0101 empty empty

