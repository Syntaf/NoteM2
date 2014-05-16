#include <windows.h>
#include <iostream>
#include "vars.h"
#include <commctrl.h>


inline BOOL LoadTextFileToEdit(HWND hEdit, LPCTSTR pszFileName)
{
    HANDLE hFile;                                               //handle to file
    BOOL bSuccess = FALSE;                                      //tests if process results in true or fals
                                                                //create file
    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, 0, NULL);
    if(hFile != INVALID_HANDLE_VALUE)                           //if file creation failed
    {
        DWORD dwFileSize;

        dwFileSize = GetFileSize(hFile, NULL);                  //if file does not exist
        if(dwFileSize != 0xFFFFFFFF)
        {
            LPSTR pszFileText;                                  //var to hold text from file
                                                                //allocate memory to hold file contents
            pszFileText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
            if(pszFileText != NULL)                             //as long as SOMETHING is in the file
            {
                DWORD dwRead;                                   //create var to read in data

                if(ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL))
                {
                    pszFileText[dwFileSize] = 0;               //add null term
                    if(SetWindowText(hEdit, pszFileText))      //set text to window
                        bSuccess = TRUE;                       //results in sucess
                }
                GlobalFree(pszFileText);                       //free memory used
            }
        }
        CloseHandle(hFile);                                     //close file
    }
    return bSuccess;
}

inline BOOL SaveTextFileFromEdit(HWND hEdit, LPCTSTR pszFileName)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwTextLength;

        dwTextLength = GetWindowTextLength(hEdit);              //retreive data from window
        if(dwTextLength > 0)
        {
            LPSTR pszText;
            DWORD dwBufferSize = dwTextLength + 1;

            pszText = (LPSTR)GlobalAlloc(GPTR, dwBufferSize);   //allocate memory to hold data
            if(pszText != NULL)                                 //as long as SOMETHING is in the file
            {
                if(GetWindowText(hEdit, pszText, dwBufferSize)) //get text from window
                {
                    DWORD dwWritten;                            //use var
                                                                //write to file
                    if(WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL))
                        bSuccess = TRUE;
                }
                GlobalFree(pszText);                            //free data
            }
        }
        CloseHandle(hFile);                                     //close file
    }
    return bSuccess;
}

inline void DoFileOpen(HWND hwnd)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));                              //set memory in struct to zero

    ofn.lStructSize = sizeof(ofn);                              //set struct members
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";

    if(GetOpenFileName(&ofn))                                   //open window for user to choose file
    {
        HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
        if(LoadTextFileToEdit(hEdit, szFileName))               //if the function we wrote above works
        {
            SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Opened");
            SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)szFileName);

            SetWindowText(hwnd, szFileName);
        }
    }
}

inline void DoFileSave(HWND hwnd)                                      //self explanatory, same as dofileopen
{
    OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "txt";
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if(GetSaveFileName(&ofn))
	{
		HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
		if(SaveTextFileFromEdit(hEdit, szFileName))
		{
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Saved...");
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)szFileName);

			SetWindowText(hwnd, szFileName);
		}
    }
}
