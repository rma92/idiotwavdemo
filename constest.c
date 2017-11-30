#include <windows.h>
#include <mmsystem.h>
#include "playwav.h"

//windres playwav.rc -O coff -o playwav.res
//gcc playwav.c playwav.res -l winmm -o playwav.exe
//tcc32 playwav.c playwav.res -lwinmm -o playwav.exe
int main()
{
  PlaySound(
    MAKEINTRESOURCE(IDI_WAV1), 
    GetModuleHandle(NULL),  
    SND_RESOURCE);
  return 0;
}
