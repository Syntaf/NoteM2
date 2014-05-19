///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+///
///                                 NOTEPAD-- v1.0                                        ///
///    Developed by Grant Mercer using Code::blocks 10.05 c++ and the Win32 library       ///
///    This program is OPEN SOURCE, feel free to use at however you wish. Bugs and        ///
///    suggestions should be sent to Gmercer015@gmail.com.                                ///
///                                                                                       ///
///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+///
#include <iostream>
#include <windows.h>
#include "resource.h"
#include <commctrl.h>
#include "hideconsole.h"

///LIBS LINKED:
    //libcomctl32.a
    //libcomdlg32.a
    //libgdi32.a
#include "file_handle.cpp"


HWND g_hMDIClient = NULL;
HWND g_hMainWindow = NULL;




///LOAD TEXT FROM FILE INTO PROGRAM
BOOL LoadTextFileToEdit(HWND hEdit, LPCTSTR pszFileName);
///SAVE FILE FROM PROGRAM
BOOL SaveTextFileFromEdit(HWND hEdit, LPCTSTR pszFileName);
///OPEN FILE
void DoFileOpen(HWND hwnd);
///SAVE FILE
void DoFileSave(HWND hwnd);

///FUNCTION TO CREATE CHILD WINDOWS
HWND CreateNewMDIChild(HWND hMDIClient)
{
    MDICREATESTRUCT mcs;                                        //struct to create child
    HWND hChild;

    mcs.szTitle = "[Untitled]";                                 //just like WINDOWEX, set struct params and info
    mcs.szClass = szChildClassName;
    mcs.hOwner = GetModuleHandle(NULL);
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
    mcs.style = MDIS_ALLCHILDSTYLES;
                                                                //create child
    hChild = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LONG)&mcs);
    if(!hChild)                                                 //if file creation fails
        MessageBox(hMDIClient, "MDI Child creation failed...Please contact developer.", "ERROR",
                   MB_ICONEXCLAMATION | MB_OK);
    return hChild;
}

///WINDOW PROC
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
            HWND hTool;
            TBBUTTON tbb[3];                                    //three buttons(save,open,new)
            TBADDBITMAP tbab;                                   //bitmaps to hold image for button

            HWND hStatus;                                       //status bar
            int statwidths[] = {100, -1};

            CLIENTCREATESTRUCT ccs;                             //create MDI Client
            ccs.hWindowMenu = GetSubMenu(GetMenu(hwnd), 2);     //find window menu where children will be listed
            ccs.idFirstChild = ID_MDI_FIRSTCHILD;

            g_hMDIClient = CreateWindowEx(WS_EX_CLIENTEDGE, "mdiclient", NULL,
                            WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL |
                            WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, hwnd, (HMENU)IDC_MAIN_MDI, GetModuleHandle(NULL), (LPVOID)&ccs);
            if(g_hMDIClient == NULL)
                MessageBox(hwnd, "Could not create MDI client...please contact developer.", "ERROR",
                           MB_OK | MB_ICONERROR);


            ///CREATE TOOLBAR
            hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                hwnd, (HMENU)IDC_MAIN_TOOL, GetModuleHandle(NULL), NULL);
            if(hTool == NULL)
                MessageBox(hwnd, "Tool bar creation failed...please contact developer.", "ERROR",
                           MB_OK | MB_ICONERROR);
                                                                    //send size of toolbar
            SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

                                                                    //use images in common control library defined by win32
            tbab.hInst = HINST_COMMCTRL;
            tbab.nID = IDB_STD_SMALL_COLOR;
			SendMessage(hTool, TB_ADDBITMAP, 0, (LPARAM)&tbab);


			ZeroMemory(tbb, sizeof(tbb));                           //zero out memory
			tbb[0].iBitmap = STD_FILENEW;                           //predefined images and info
			tbb[0].fsState = TBSTATE_ENABLED;                       //button enabled(can be clicked)
			tbb[0].fsStyle = TBSTYLE_BUTTON;                        //style: its a button
			tbb[0].idCommand = ID_FILE_NEW;                         //command it sends when pressed

			tbb[1].iBitmap = STD_FILEOPEN;                          //etc... etc..
			tbb[1].fsState = TBSTATE_ENABLED;
			tbb[1].fsStyle = TBSTYLE_BUTTON;
			tbb[1].idCommand = ID_FILE_OPEN;

			tbb[2].iBitmap = STD_FILESAVE;
			tbb[2].fsState = TBSTATE_ENABLED;
			tbb[2].fsStyle = TBSTYLE_BUTTON;
			tbb[2].idCommand = ID_FILE_SAVEAS;

                                                                    //send message to add buttons to main window
			SendMessage(hTool, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);

                                                                    // Create Status bar(bottom of screen, cosmetic)
			hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
				hwnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);

                                                                    //give status bar 2 parts
			SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
                                                                    //set initial text
			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"NoteM2");
		}
		break;
		case WM_SIZE:
		{
			HWND hTool;                                             //tool bar
			RECT rcTool;                                            //rect to hold tool bar
			int iToolHeight;                                        //height of tool bar

			HWND hStatus;                                           //status bar
			RECT rcStatus;                                          //area of status bar
			int iStatusHeight;                                      //height

			HWND hMDI;                                              //current child window
			int iMDIHeight;                                         //height of window
			RECT rcClient;                                          //area of edit control

                                                                    // Size toolbar and get height

			hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);                //find item
			SendMessage(hTool, TB_AUTOSIZE, 0, 0);                  //auto size tool bar for user

			GetWindowRect(hTool, &rcTool);                          //set area from autosize
			iToolHeight = rcTool.bottom - rcTool.top;               //find height of toolbar

			// Size status bar and get height

			hStatus = GetDlgItem(hwnd, IDC_MAIN_STATUS);            //etc...etc....
			SendMessage(hStatus, WM_SIZE, 0, 0);

			GetWindowRect(hStatus, &rcStatus);
			iStatusHeight = rcStatus.bottom - rcStatus.top;

			// Calculate remaining height and size edit

			GetClientRect(hwnd, &rcClient);

			iMDIHeight = rcClient.bottom - iToolHeight - iStatusHeight;

			hMDI = GetDlgItem(hwnd, IDC_MAIN_MDI);
			SetWindowPos(hMDI, NULL, 0, iToolHeight, rcClient.right, iMDIHeight, SWP_NOZORDER);
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);                                    //destroy window
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		case WM_COMMAND:                                            //note** this could be sent via menu OR buttons, the button
			switch(LOWORD(wParam))                                  //the exact same message as the menu items do
			{
				case ID_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);              //close window
				break;
				case ID_FILE_NEW:
					CreateNewMDIChild(g_hMDIClient);                //create )NEW(child window
				break;
				case ID_FILE_OPEN:
				{
				    HWND hChild = CreateNewMDIChild(g_hMDIClient);  //create child window
				    if(hChild)                                      //if there is a child window open
                        DoFileOpen(hChild);                           //call open function
				}
				break;
				case ID_FILE_CLOSE:
				{
                                                                    //create handle to current child window
				    HWND hChild = (HWND)SendMessage(g_hMDIClient, WM_MDIGETACTIVE,0 ,0);
				    if(hChild)                                      //if there is actually a window open
				    {
				        SendMessage(hChild, WM_CLOSE, 0, 0);        //send message to close child
				    }
				}
				break;
				case ID_WINDOW_TILE:
                    SendMessage(g_hMDIClient, WM_MDITILE, 0, 0);
                break;
                case ID_WINDOW_CASCADE:
                    SendMessage(g_hMDIClient, WM_MDICASCADE, 0, 0);
                break;
                default:                                            //handle Child commands
                {
                    if(LOWORD(wParam) >= ID_MDI_FIRSTCHILD)
                    {
                                                                    //if user clicks on a Child window, send
                                                                    //to frame proc to handle
                        DefFrameProc(hwnd, g_hMDIClient, WM_COMMAND, wParam, lParam);
                    }
                    else
                    {
                                                                    //if not, send the command to the child
                                                                    //window for processing
                        HWND hChild = (HWND)SendMessage(g_hMDIClient, WM_MDIGETACTIVE, 0, 0);
                        if(hChild)
                            SendMessage(hChild, WM_COMMAND, wParam, lParam);
                    }
                }
			}
		break;
		default:
                                                                     //adds specialized handling for frame windows
			return DefFrameProc(hwnd, g_hMDIClient, msg, wParam, lParam);
	}
	return 0;
}

///CHILD PROC
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
                                                                    //create child window
            HFONT hfDefault;                                        //create font var and
            HWND hEdit;                                             //Edit window var

            hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",    //create window
                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                    ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0,
                    0, 100, 100, hwnd, (HMENU)IDC_CHILD_EDIT, GetModuleHandle(NULL), NULL);
            if(hEdit == NULL)                                       //if edit window fails, tell user
                std::cerr << "Could not create edit box...";

            hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);    //find font
                                                                    //set font for program
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        break;
        case WM_MDIACTIVATE:
        {
            HMENU hMenu, hFileMenu;                                 //menu handles
            UINT EnableFlag;

            hMenu = GetMenu(g_hMainWindow);
            if(hwnd == (HWND)lParam)                                //enable menus
                EnableFlag = MF_ENABLED;
            else
                EnableFlag = MF_GRAYED;                             //deactivate(grey) menus

            EnableMenuItem(hMenu, 1, MF_BYPOSITION | EnableFlag);   //enable grayed out menus
            EnableMenuItem(hMenu, 2, MF_BYPOSITION | EnableFlag);

            hFileMenu = GetSubMenu(hMenu, 0);
                                                                    //enable features that can be used now
            EnableMenuItem(hFileMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | EnableFlag);

            EnableMenuItem(hFileMenu, ID_FILE_CLOSE, MF_BYCOMMAND | EnableFlag);
            EnableMenuItem(hFileMenu, ID_FILE_CLOSEALL, MF_BYCOMMAND | EnableFlag);

            DrawMenuBar(g_hMainWindow);                             //redraw menu
        }
        break;
        case WM_COMMAND:
            switch(LOWORD(wParam))                                  //search through child commands
            {
                case ID_FILE_OPEN:
                    DoFileOpen(hwnd);                               //open file
                break;
                case ID_FILE_SAVEAS:
                    DoFileSave(hwnd);                               //save file
                break;
                case ID_EDIT_CUT:                                   //controls, edit, paste, copy
                    SendDlgItemMessage(hwnd, IDC_CHILD_EDIT, WM_CUT, 0, 0);
                break;
                case ID_EDIT_COPY:
                    SendDlgItemMessage(hwnd, IDC_CHILD_EDIT, WM_COPY, 0, 0);
                break;
                case ID_EDIT_PASTE:
                    SendDlgItemMessage(hwnd, IDC_CHILD_EDIT, WM_PASTE, 0, 0);
                break;
            }
        break;
        case WM_SIZE:
        {
            HWND hEdit;                                             //handle to edit and client to determine size
            RECT rcClient;

            GetClientRect(hwnd, &rcClient);

            hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
            SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
        }
        return DefMDIChildProc(hwnd, msg, wParam, lParam);          //return proc
        default:
            return DefMDIChildProc(hwnd, msg, wParam, lParam);      //return proc
    }
    return 0;
}

BOOL SetUpMDIChildWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wc;                                                  //create window class
    ///DEFINE CHILD WINDOW CLASS
    wc.cbSize               = sizeof(WNDCLASSEX);                   //size of class
    wc.style                = CS_HREDRAW | CS_VREDRAW;              //style
    wc.lpfnWndProc          = MDIChildWndProc;                      //proc
    wc.cbClsExtra           = 0;                                    //extra memeory, dont need extra
    wc.cbWndExtra           = 0;
    wc.hInstance            = hInstance;                            //handle
    wc.hIcon                = LoadIcon(NULL, IDI_APPLICATION);      //icon
    wc.hCursor              = LoadCursor(NULL, IDC_ARROW);          //cursor
    wc.hbrBackground        = (HBRUSH)(COLOR_3DFACE+1);             //backgroud(white)
    wc.lpszMenuName         = NULL;                                 //no menu
    wc.lpszClassName        = szChildClassName;                     //name of class
    wc.hIconSm              = LoadIcon(NULL, IDI_APPLICATION);      //small icon

    if(!RegisterClassEx(&wc))
    {
        std::cerr << "Could not register child window..";
        return FALSE;
    }
    else
        return TRUE;                                                //function success
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	InitCommonControls();                                           //initialize common controls(for buttons)

    ///DEFINE WINDOW CLASS
	wc.cbSize		 = sizeof(WNDCLASSEX);                          //size
	wc.style		 = 0;                                           //no style
	wc.lpfnWndProc	 = WndProc;                                     //proc
	wc.cbClsExtra	 = 0;                                           //no extra memory needed
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;                                   //handle to program
                                                                    //Load custom icon(v1.0)
	wc.hIcon		 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);                 //cursor
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);                    //set background
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);               //get menu from resource file
	wc.lpszClassName = szClassName;                                 //class name
                                                                    //Load custom icon(v1.0)
	wc.hIconSm		 = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 16, 16, 0);

	if(!RegisterClassEx(&wc))                                       //if class fails register
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",   //warn user
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	if(!SetUpMDIChildWindowClass(hInstance))                        //if MDI child failure, warn user
        return 0;


    ///CREATE MAIN WINDOW
	hwnd = CreateWindowEx(
		0,
		szClassName,
		"NoteM2",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 480, 640,
		NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	g_hMainWindow = hwnd;

    ShowWindow( GetConsoleWindow(), SW_HIDE );                                                                //show and update window
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

    ///MAIN LOOP
	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
	    if(!TranslateMDISysAccel(g_hMDIClient, &Msg))
	    {
	        TranslateMessage(&Msg);
            DispatchMessage(&Msg);
	    }
	}
	return Msg.wParam;
}
