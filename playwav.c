#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h> //this is needed for atof in the svg drawing.
#include "playwav.h"

//uncomment this to have svg path printing
//#define DBGPRINT

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

//parse an svg path and draw it.  
//x and y is the top left corner.
//scaleX and scaleY are multipliers
//supports:
//
void drawPath( HDC hdc, char * c, int loc_x, int loc_y, float scale, int max_str_length )
{
  int i; //index of characters.
  int l_last; //lexer - index of the last character.
  int tx, ty;
  int* tokens = calloc( 1024 , sizeof( int ) );
  //tokens will contain a number for a parameter (which we will & with 0xFFFF - there shouldn't be any numbers larger than that.)  We can multiply by scaleX at this time to do floating math.
  //commands will contain the char of the command, we will | with 0x00FF0000. 
  //when its time to parse, see if ( value & 0x00FF0000 ) is true to see if we have command or value.
  //If the command is longer than 2 characters, only the last two characters will be stored.
  //  (SVG commands are usually only one letter)
  int num_tokens = 0;
  int has_command = 0;
  int has_value = 0;
  char temp_char = '\0';
  float temp_float = 0.0f;
  int first_x = -1;
  int first_y = -1;
  int j;//temp variable only for printing.

  //lexer: Iterate over the string, when a space (or comma) is encountered,
  //  determine if the last token was a command, or a number (only . and numbers).
  //  if while looking over the word a letter is found, then the word is a command.
  //  if a number is found then  it is a number.
  //  when the space is found, it will be stored as appropriate.  If both number
  //  and letter have been found, it will be treated as a command. (since those
  //  are whitelisted).
  for( i = 0, l_last = 0; i < max_str_length; ++i )
  {
    if( (has_value | has_command) && (c[i] == ',' || c[i] == ' ' || c[i] == '\0') )
    {
      /*
      //debug printing
      printf("Found %s:", (has_command)?"command":"value" );
      
      for( j = l_last; j < i; ++j )
      {
        printf("%c", c[j]);
      }
      printf("$\n");
      //end debug printing
      */

      /*
        if there is a letter in this word 2 characters ago,
          store it,
          left shift so that it's in the 2nd least significant byte.
        if i >= 1 (to prevent anything stupid)
          set the least significant byte to the letter.
        or with 0xFF0000 and store.
      */
      if( has_command )
      { 
        j = 0;
        if( i >= 2 && i - 2 >= l_last )
        {
          j |= c[i - 2];
          j <<= 8;
        }
        if( i >= 1 )
        {
          j |= c[i - 1];
        }
        tokens[ num_tokens ] = 0xFF0000 | j;
      }
      /*
        Temporarily set the character after to null.
        Call atof on the pointer, arithmaticed to the start of the 
          number in the string.
        Round the float, store it in j.
        Put the altered character back to the original one.
        add j to the tokens[ num_tokens ]
      */
      else if( has_value )
      {
        temp_char = c[i];
        c[i] = '\0';
        temp_float = atof( c+l_last );
        temp_float *= scale;
        j = (int)temp_float;
        if( temp_float - j >= 0.5 ) ++j;
        //printf("float = %f\n", temp_float );
        c[i] = temp_char;
        tokens[ num_tokens ] = 0xFFFF & j;
      }
      
      l_last = i + 1;
      has_value = 0;
      has_command = 0;
      ++num_tokens;
    }
    else if( (c[i] <= '9' && c[i] >= '0' ) || c[i] == '.' )
    { //needs to be before the letter checking.
      has_value = 1;
    }
    else if( c[i] | 0x20 == 'm' || c[i] | 0x20 == 'l' || c[i] | 0x20 == 'z' )
    {
      has_command = 1;
    }
    else if( c[i] == '\0' )
    {
      break;
    }
  }//lexer for loop

  //Debug printing tokens:
#ifdef DBGPRINT
  printf("\n\ntokens:\n");
  for( i = 0; i < num_tokens; ++i )
  {
    if( tokens[ i ] & 0xFF0000 )
    {
      printf("cmd: %c%c\n", (tokens[i] & 0xff00) >> 8, tokens[i] & 0xff);
    }
    else
    {
      printf("val: %d\n", tokens[i]) ;
    }
  }
  printf("\n\n");
#endif DBGPRINT

  //Drawing
  //Reset variables
  i = 0;
  j = 0;
  tx = 0;
  ty = 0;

  temp_char = 0;//will hold command
  //BeginPath( hdc );
  //TODO: finish Z, relative movements,
  //   and set up x and y adjust and scale.
  //MoveToEx( hdc, x, y, 0);
  
  for( i = 0; i < num_tokens; ++i )
  {
    if( tokens[ i ] & 0xFF0000 )  //if is command?
    {
      temp_char = tokens[ i ] & 0xFF;
      switch( temp_char )
      {
        case 'M': //move to absolute
          tx = tokens[ ++i ];
          ty = tokens[ ++i ];
          if( i >= num_tokens )
          {
            printf("error: wrong number of parameters!\n");
            return;
          }
          if( first_x < 0 && first_y < 0 )
          {
            first_x = tx;
            first_y = ty;
          }
#ifdef DBGPRINT
          printf("MoveTo ABS: %d %d\n", tx, ty);
#endif DBGPRINT
          BeginPath( hdc );
          MoveToEx( hdc, loc_x + tx, loc_y + ty, 0 );//we don't need to store the old point.
        break;
        case 'L': //line to absolute
          tx = tokens[ ++i ];
          ty = tokens[ ++i ];
          if( i >= num_tokens )
          {
            printf("error: wrong number of parameters!\n");
            return;
          }
#ifdef DBGPRINT
          printf("LineTo ABS: %d %d\n", tx, ty);
#endif DBGPRINT
          LineTo( hdc, loc_x + tx, loc_y + ty );
          break;
        case 'Z':
        case 'z':
#ifdef DBGPRINT
          printf("z: %d %d", first_x, first_y );
#endif DBGPRINT
          LineTo( hdc, loc_x + first_x, loc_y +first_y);
          break;
        default:
          printf("unknown command \"%c\"\n", temp_char );
      }
    }
  }

  EndPath( hdc );
  FillPath( hdc );

  free( tokens );
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
      HBRUSH hPurBrush = CreateSolidBrush( RGB(128,0,128) );
      HBRUSH hGreenBrush = CreateSolidBrush( RGB( 0, 255, 0 ) );
      HBRUSH hTempBrush;
      hdc = BeginPaint( hWnd, &ps );
      paint( hWnd, hdc);
      
      //an actual drawing
      hTempBrush = SelectObject( hdc, hPurBrush );
      //drawPath( hdc, "M 100.376 100.9 L 200 60 L 300 100 L 300 300 L 100 300 z", 20, 20, 1.0f, 255);
      drawPath( hdc, "M 608.4,367.9 L 474,3.4 L 400.4,3.4 L 391,170.3 L 255,0 L 83.9,0 L 41.1,8.6 L 24.8,24 L 3.4,50.5 L 0,82.1 L 2.6,109.5 L 14.5,136.9 L 27.4,160.9 L 51.3,181.4 L 72.7,201.1 L 97.5,213 L 86.4,364.5 L 172.8,362.8 L 178,225 L 258.4,225 L 309.7,364.5 L 386.8,364.5 L 338.8,225.9 L 451.8,367.1 L 462.1,367.1 L 468.9,365.3 L 474,217.3 L 531.4,367.9 L 608.4,367.9 L 608.4,367.9",
                80, 20, 1.5f, 2048);
      SelectObject( hdc, hGreenBrush );
      drawPath( hdc, "M 235,149.5 L 209.4,77.4 L 107.9,77.4 L 88.9,85 L 85.1,96.4 L 87,108.7 L 92.7,121.1 L 103.2,134.3 L 117.4,143.8 L 126.9,147.6 L 235,149.5 L 235,149.5",
                80, 20, 1.5f, 2048);
      SelectObject( hdc, hTempBrush );
      DeleteObject( hTempBrush );
      
      //Testing the lexer.
      //drawPath( hdc, "M 100.376 100.9 c3 LongCommand 32 L 300 100 L 200 300 z", 20, 20, 1.0f, 1.0f, 255);
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

