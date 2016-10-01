#include "auxgv.h"
#include "SetFSProc.h"
#include "auxgvDlg.h"

#include "dlgs.h"

#define		WM__INIT2			WM_APP+100

CSetFS dlg;

CAuxDlg *hPar;

/*
UINT_PTR CALLBACK ChooseFontHookProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int id, fontsize;
	HDC hdc;
	char buf[64];
	CHOOSEFONT   *tpchf;
    switch (message)
    {
		

        case WM_INITDIALOG:
			tpchf = (CHOOSEFONT*)lParam;
            if (chf.Flags & CF_ENABLETEMPLATE)
            {
                ShowWindow(GetDlgItem(hwnd, stc4), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, cmb4), SW_HIDE);
            }

			//Try to do the below without processing WM_INITDIALOG message.  Maybe WM_TIMER? or something else?


			// cmb1 font combo box

			id = SendDlgItemMessage(hwnd, cmb1, CB_FINDSTRINGEXACT, -1, (LPARAM)chf.lpLogFont->lfFaceName);
			SendDlgItemMessage(hwnd, cmb1, CB_SETCURSEL, id, NULL);
			hdc = GetDC(NULL);
			fontsize = -chf.lpLogFont->lfHeight * 72 / GetDeviceCaps(hdc, LOGPIXELSY);
			ReleaseDC(NULL, hdc);
			sprintf(buf, "%d", fontsize);
			id = SendDlgItemMessage(hwnd, cmb3, CB_FINDSTRINGEXACT, -1, (LPARAM)buf);
			SendDlgItemMessage(hwnd, cmb1, CB_SETCURSEL, id, NULL);
	        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                if (MessageBox(hwnd, "Are you sure you want to change the font?",
                    "Information", MB_YESNO ) == IDYES )
                    break;
                return (TRUE);

            }
            break;
    }
    return (FALSE);
}
*/
CSetFS::CSetFS()
:font_changed(false)
{

}

CSetFS::~CSetFS()
{

}

BOOL CSetFS::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	lpchf = (CHOOSEFONT *)lParam;
	GetParent(hDlg);
	hPar = (CAuxDlg *)lpchf->lCustData;
	hInst = lpchf->hInstance;
	udfpath = hPar->udfpath;
	fontsize = hPar->fontsize;
	lf = hPar->lf;
	PostMessage(WM__INIT2, (WPARAM)(fs = hPar->fs));
	return 1;
}

void CSetFS::InitDialog2(int dummy)
{
	HDC hdc;
	SetDlgItemText(IDC_UDFPATH, udfpath.c_str());

	hdc = GetDC(NULL);
	lf.lfHeight = -MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
	SetDlgItemInt(IDC_FONTSIZE, fontsize);
	SetDlgItemText(IDC_FONTNAME, lf.lfFaceName);
	SetDlgItemInt(IDC_FS, fs);
}

void CSetFS::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	int k, k2, res;
	HDC hdc;
	char buf[2048];
	switch (idc)
	{
	//case IDC_CHANGEFONT:
 //       font_changed = ChooseNewFont();
	//	hdc = GetDC(NULL);
	//	fontsize = -lf.lfHeight * 72 / GetDeviceCaps(hdc, LOGPIXELSY);
	//	ReleaseDC(NULL, hdc);
	//	SetDlgItemInt(IDC_FONTSIZE, fontsize);
	//	SetDlgItemText(IDC_FONTNAME, lf.lfFaceName);
	//	break;

	case IDOK:
		res = GetDlgItemInt(IDC_FS, &k);
		GetDlgItemText(IDC_UDFPATH, buf, sizeof(buf));
		if (k)
		{
			fs=hPar->fs=res;
			udfpath = buf;
			hPar->udfpath = buf;
			fontsize = GetDlgItemInt(cmb3, &k2);
			hdc = GetDC(NULL);
			lf.lfHeight = -MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			ReleaseDC(NULL, hdc);
			GetDlgItemText(cmb1,buf, sizeof(buf));
			strcpy(lf.lfFaceName,buf);
			hPar->lf = lf;
			::SendMessage(lpchf->hwndOwner,WM__FONTCHANGED, 0, 0);
			EndDialog(hDlg, 0);
		}
		else
			MessageBox("Invalid sample rate");
		break;
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		break;
	}
}

void CSetFS::OnClose()
{
	PostMessage(WM_COMMAND,IDABORT);
}

BOOL CALLBACK  SettingsHookProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	dlg.hDlg = hDlg;
	switch (umsg)
	{
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, dlg.OnCommand);
	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, dlg.OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_DESTROY, dlg.OnClose);
	chHANDLE_DLGMSG (hDlg, WM__INIT2, dlg.InitDialog2);
		break;
	default:		return 0;
	}
	return 1;
}
