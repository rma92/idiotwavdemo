windres -O coff playwav.rc -o playwav.res
tcc playwav.c playwav.res -lwinmm -o idiot.exe
REM gcc playwav.c playwav.res -lwinmm -lgdi32 -o playwav.exe
REM make an 8kbps mp3 using Audition
REM ffmpeg -i input.mp3 -c copy -f wav embedded_mp3.wav
upx --ultra-brute -9 idiot.exe
