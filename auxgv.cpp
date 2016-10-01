#include "auxgv.h"
#include "auxgvdlg.h"
#include <stdlib.h>
#include <process.h>

HINSTANCE hInst;
HWND hAuxDlg;
char ApplicationPath[MAX_PATH];

CWndDlg wnd;
CAuxDlg auxMain;

map<HWND,HANDLE> fig_hDlg;

map<unsigned int, string> wmstr;
AUDFRET_EXP void makewmstr(map<unsigned int, string> &wmstr);



BOOL CALLBACK auxDlg (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	switch (umsg)
	{
	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, wnd.OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_SIZE, auxMain.OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, auxMain.OnClose);
	chHANDLE_DLGMSG (hDlg, WM_DESTROY, auxMain.OnDestroy);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, auxMain.OnCommand);
	chHANDLE_DLGMSG (hDlg, WM_CTLCOLORSTATIC, auxMain.OnCtlColorStatic);
	chHANDLE_DLGMSG (hDlg, WM_CTLCOLOREDIT, auxMain.OnCtlColorEdit);
	chHANDLE_DLGMSG (hDlg, WM__FONTCHANGED, auxMain.FontChanged);
	default:
		return FALSE;
	}
	return TRUE;
}

#define SHOWFIGDLG \


int  WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	MSG         msg ;
	HACCEL hAccel;

	auxMain.hInst = hInst = hInstance;

	wnd.hDlg = CreateDialog (hInst, MAKEINTRESOURCE(IDD_AUXGV_DIALOG), NULL, (DLGPROC)auxDlg);
	hAuxDlg = wnd.hDlg;

	HANDLE h = LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0);
#ifdef _WIN64
	SetClassLongPtr (hAuxDlg, GCLP_HICON, (LONG)(LONG_PTR)h);
#else
	SetClassLongPtr (hAuxDlg, GCL_HICON, (LONG)(LONG_PTR)h);
#endif
	auxMain.hDlg = wnd.hDlg ;
	strcpy(auxMain.AppPath, wnd.AppPath);
	auxMain.OnInitDialog(NULL, NULL);
	ShowWindow(auxMain.hDlg, iCmdShow);
	UpdateWindow (auxMain.hDlg);
	hAccel = LoadAccelerators (auxMain.hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	while (GetMessage (&msg, NULL, 0, 0))
	{ 
		int res1(0);
		bool loop(true);
		//if  msg.hwnd is one of the figure windows, or msg.hwnd is the control window of such figure window
		for (map<HWND,HANDLE>::iterator it=fig_hDlg.begin(); it!=fig_hDlg.end() && loop; it++)
		{
			if (it->first==msg.hwnd || it->first==GetParent(msg.hwnd))
			{
				loop = false;
				if (msg.message==WM_DESTROY)	
				{
					res1=1;
					deleteObj(it->second);
				}
				else
				{
					if (it->first==msg.hwnd)	res1=TranslateAccelerator(msg.hwnd, auxMain.hAccel, &msg);
					else			 res1=TranslateAccelerator(GetParent(msg.hwnd), auxMain.hAccel, &msg);
				}
			}
		}
		if (!res1)
		{
			if(!TranslateAccelerator (auxMain.hDlg, hAccel, &msg))
			{
				if (!IsDialogMessage(auxMain.hDlg, &msg))
				{
					TranslateMessage (&msg) ;
					DispatchMessage (&msg) ;
				}
			}
		}
	}
	return (int)msg.wParam ;
}


