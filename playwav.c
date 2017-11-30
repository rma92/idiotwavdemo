#include <windows.h>
#include <mmsystem.h>
#include "playwav.h"

int counter = 0;
WCHAR * str = NULL;
void paint( HWND hWnd, HDC hdc )
{
  RECT clientRect, temp;
  HBRUSH hBackBrush;
  HFONT hFontOld;
  HFONT hFont = CreateFont(SETTING_LINE_HEIGHT, 0, 0, 0, 300, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "times");
  
  if( counter & 1 ) 
  {
    hBackBrush = CreateSolidBrush( RGB(255,255,255) );
    SetTextColor( hdc, 0x0);
  }
  else
  {
    hBackBrush = CreateSolidBrush( RGB(0,0,0) );
    SetTextColor( hdc, 0xFFFFFF);
  }
  SetBkMode(hdc,TRANSPARENT);
  
  GetClientRect( hWnd, &clientRect );
  FillRect( hdc, &clientRect, hBackBrush );

  temp = clientRect;

  temp.top = (clientRect.bottom - SETTING_LINE_HEIGHT * 3 ) / 2;
  wsprintfW (str, L"you are an idiot");
  hFontOld = SelectObject( hdc, hFont );
  DrawTextW( hdc, str, -1, &temp, DT_SINGLELINE | DT_CENTER | DT_WORDBREAK);

  wsprintfW (str, L"%c %c %c", 0x263A, 0x263A, 0x263A );
  temp.top += SETTING_LINE_HEIGHT * 2;
  DrawTextW( hdc, str, -1, &temp, DT_SINGLELINE | DT_CENTER | DT_WORDBREAK);

  SelectObject( hdc, hFontOld);
  DeleteObject( hBackBrush );
  DeleteObject( hFont );

  return;
}

DWORD WINAPI MyThreadFunction( LPVOID lpParam )
{
  while(1)
  {
    PlaySound(
      MAKEINTRESOURCE(IDI_WAV1), 
      GetModuleHandle(NULL),  
      SND_RESOURCE);
    Sleep(500);
  }
  return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_CREATE:
      SetTimer( hWnd, IDT_TIMER1, SETTING_INTERVAL_FLASH, (TIMERPROC) NULL );
      CreateThread( NULL, 0, MyThreadFunction, 0, 0, 0 );
      str = (WCHAR*)calloc( 255, sizeof( WCHAR ) );
      break;
    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      hdc = BeginPaint( hWnd, &ps );
      paint( hWnd, hdc);
      EndPaint( hWnd, &ps );
    }
      break;
    case WM_TIMER:
      switch( wParam )
      {
        case IDT_TIMER1:
        {
          ++counter;
          InvalidateRect( hWnd, 0, TRUE );
          if( counter > 127 )
          {
            counter = 0;
          }
        }
        return 0;
      }
    break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  MSG msg1;
  WNDCLASS wc1;
  HWND hWnd1;
  ZeroMemory(&wc1, sizeof wc1);
  wc1.hInstance = hInst;
  wc1.lpszClassName = AppName;
  wc1.lpfnWndProc = (WNDPROC)WndProc;
  wc1.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
  wc1.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
  wc1.hIcon = LoadIcon(NULL, IDI_INFORMATION);
  wc1.hCursor = LoadCursor(NULL, IDC_ARROW);
  if(RegisterClass(&wc1) == FALSE) return 0;
  hWnd1 = CreateWindow(AppName, AppName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 10, 10, 360, 240, 0, 0, hInst, 0);
  if(hWnd1 == NULL) return 0;
  ShowWindow( hWnd1, SW_MAXIMIZE);
  while(GetMessage(&msg1,NULL,0,0) > 0){
    TranslateMessage(&msg1);
    DispatchMessage(&msg1);
  }
  return msg1.wParam;
}

