#include <ctime>
#include <windows.h>

#if _WIN32_WINNT < 0x0501

typedef HWND WINAPI (*PGetConsoleWindow)();
PGetConsoleWindow GetConsoleWindow = NULL;

HWND WINAPI MyGetConsoleWindow()
  {
  HWND result;
  char save_title[ 1000 ];
  char temp_title[ 50 ];
  time_t t = time( NULL );
  lstrcpyA( temp_title, "madmaxsantana " );
  lstrcatA( temp_title, ctime( &t ) );

  GetConsoleTitleA( save_title, sizeof( save_title ) );
  SetConsoleTitleA( temp_title );
  result = FindWindowA( NULL, temp_title );
  SetConsoleTitleA( save_title );

  return result;
  }

struct MyInitializeConsole
  {
  MyInitializeConsole()
    {
    HINSTANCE kernel32 = LoadLibraryA( "Kernel32.dll" );
    GetConsoleWindow =
      (PGetConsoleWindow)GetProcAddress( kernel32, "GetConsoleWindow" );
    if (GetConsoleWindow == NULL)
      GetConsoleWindow = &MyGetConsoleWindow;
    FreeLibrary( kernel32 );
    }
  }
  MyInitializedConsole;

#endif
