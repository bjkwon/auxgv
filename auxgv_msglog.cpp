// This is identical to the auxgv.cpp of version auxviewer 1.41 except that this provides msg log output.
// This is the most stable version of graffy with multiple figure windows 
// 9-26-2016 bjk

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
FILE *fp;


BOOL CALLBACK auxDlg (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	if (umsg==WM_COMMAND ||umsg==WM_SYSCOMMAND)
	{
		fp=fopen("log.txt","at"); 
		fprintf(fp,"%s, wparam=(%d,%d), lParam=%x\n",wmstr[umsg].c_str(), HIWORD(wParam), LOWORD(wParam), lParam);
		fclose(fp);
	}
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

	makewmstr(wmstr);
	fp=fopen("log.txt","wt");
	fprintf(fp,"main Dlg = %x\n", auxMain.hDlg);
	fclose(fp);
	char msgbuf[32], buf[256], buf1[256];

	while (GetMessage (&msg, NULL, 0, 0))
	{ 
		
		int res1(0);
		bool loop(true);

		strcpy(buf1,"fig_hDlg=[");
		for (map<HWND,HANDLE>::iterator it=fig_hDlg.begin(); it!=fig_hDlg.end(); it++)
		{
			sprintf(buf, "%x,", *it);
			strcat(buf1, buf);
		}
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
					if(wmstr[msg.message].size()>0) strcpy(msgbuf,wmstr[msg.message].c_str()); 
					else sprintf(msgbuf, "0x%04x", msg.message);
					if (res1)	{fp=fopen("log.txt","at"); fprintf(fp,"%x %s Accel translated \n", msg.hwnd, msgbuf); fclose(fp);}
				}
			}
		}
		if (!res1)
		{
			if(wmstr[msg.message].size()>0) strcpy(msgbuf,wmstr[msg.message].c_str()); else sprintf(msgbuf, "0x%04x", msg.message);
			if (msg.message<0x8000 && msg.message!=WM_MOUSEMOVE && msg.message!=WM_NCMOUSEMOVE && msg.message!=WM_TIMER && msg.message!=WM_PAINT)
			{
				if (auxMain.hDlg==msg.hwnd) sprintf(buf, "Main: %s \n", msgbuf);
				else
				{
					sprintf(buf, "%s], %x(ID:%d): %s \n", buf1, msg.hwnd, GetWindowLongPtr(msg.hwnd, GWLP_ID), msgbuf);
				}
				fp=fopen("log.txt","at"); fprintf(fp, buf); fclose(fp);
			}
			if(!TranslateAccelerator (auxMain.hDlg, hAccel, &msg))
			{
				if (!IsDialogMessage(auxMain.hDlg, &msg))
				{
//					fprintf(fp,"IsDialogMessage=N\n");
					TranslateMessage (&msg) ;
					DispatchMessage (&msg) ;
				}
//				else
//					fprintf(fp,"IsDialogMessage=Y\n");
			}
			else
			{	fp=fopen("log.txt","at"); fprintf(fp,"translated \n"); fclose(fp);}
		}
	}
	return (int)msg.wParam ;
}


