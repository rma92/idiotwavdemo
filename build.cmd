windres -O coff playwav.rc -o playwav.res
tcc playwav.c playwav.res -lwinmm -o playwav_tcc.exe
REM gcc playwav.c playwav.res -lwinmm -lgdi32 -o playwav.exe
