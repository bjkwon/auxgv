#include "auxgv.h"
#include "auxgvDlg.h"

HWND hPlotDlgCurrent;

extern  map<HWND,HANDLE> fig_hDlg;


int FileDlg(CFileDlg fdlg, char *fullfname, char *fname, int idc)
{
	switch (idc)
	{
	case IDC_OPEN:
		return fdlg.FileOpenDlg(fullfname, fname, "text file (*.TXT)\0*.txt\0", "txt");
		break;
	case IDC_SAVE:
		return fdlg.FileSaveDlg(fullfname, fname, "text file (*.TXT)\0*.txt\0", "txt");
		break;
	}
	return 0;
}
CAuxDlg::CAuxDlg(void)
{
}

CAuxDlg::~CAuxDlg(void)
{
}

BOOL CAuxDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	char errstr[256], buf[256];
	string readstr, temp;
	temp = AppPath;
	hPlotButton = GetDlgItem(IDC_MAKE_PLOT);
	hAUXInput = GetDlgItem(IDC_AUX);
	GetModuleFileName(hInst, buf, sizeof(buf));
	getVersionStringAndUpdateTitle (hDlg, buf, VerStr,sizeof(VerStr));

	sprintf(buf, "%s%s", AppPath, INIFILE);
	readINI(buf, errstr);

    HDC hdc = GetDC(NULL);
	LONG lfHeight = MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

	if (udfpath.length()>0) { temp += ';'; 	temp += udfpath; }
	AstSig.SetPath(temp.c_str());

	COLORREF col = RGB(190,230, 130);
	hEditBrush = CreateSolidBrush (col);
	return 1;
}

void CAuxDlg::FontChanged()
{
	HDC hdc;
	DeleteObject(eFont);
	hdc = GetDC(NULL);
	fontsize = -lf.lfHeight * 72 / GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	eFont = CreateFont(lf.lfHeight,0,0,0, FW_MEDIUM, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		FIXED_PITCH | FF_MODERN, lf.lfFaceName);
	::SendMessage(hAUXInput,WM_SETFONT,(WPARAM)eFont,1); 
}

void CAuxDlg::OnSize(UINT state, int cx, int cy)
{
	::MoveWindow (hAUXInput, 6, 38, cx-20, cy - 114, FALSE); // got these numbers thru trial-and-error
	::MoveWindow (hPlotButton, 6, cy - 72, cx-20, 58, TRUE); // got these numbers thru trial-and-error
}


void CAuxDlg::OnClose()
{
	::DestroyWindow(hDlg);
}

void CAuxDlg::OnDestroy()
{
	char errstr[256], buf[256], msg[256];
	DeleteObject (eFont);
	DeleteObject (hEditBrush);
	sprintf(buf, "%s%s", AppPath, INIFILE);
	if(!writeINI(buf, errstr))
	{
		sprintf(msg, "Exiting.... %s (%s)", errstr, buf);
		MessageBox(msg);
	}
	for (int i=0; i<nFigureWnd; i++) delete figwnd[i];
	delete[] figwnd;
	PostQuitMessage(0);
}

bool CAuxDlg::PrepareSig()
{
	try {
		string tp(AppPath);
		tp += ';';
		udfpath.insert(0,tp);
		AstSig.Reset(fs,udfpath.c_str());
		AstSig.SetNewScript(m_auxstr.c_str());
		Sig = AstSig.Compute();
		Sig.MakeChainless();
	} catch (const char *errmsg) {
		MessageBox(errmsg, "Error in AUX script", MB_ICONERROR);
		return false;
	}
	return true;
}


int CAuxDlg::readINI(char *fname, char *estr)
{
	char errStr[256];
	string strRead;
	int val;
	double dval;
	HDC hdc;
	int res = ReadINI (errStr, fname, "SAMPLE RATE", strRead);
	if (res>0 && sscanf(strRead.c_str(), "%d", &val)!=EOF && val >10)	fs = val;
	else																fs = DEFAULT_FS;
	res = ReadINI (errStr, fname, "PLAYBACK BLOCK SIZE MILLISEC", strRead);
	if (res>0 && sscanf(strRead.c_str(), "%f", &dval)!=EOF && dval >10.)	block = val;
	else																block = DEFAULT_BLOCK_SIZE;

	if (ReadINI (errStr, fname, "FONT FACE", strRead)>0)
		strcpy(lf.lfFaceName, strRead.c_str());
	else							
		strcpy(lf.lfFaceName,"SYSTEM"); // If font name is not specified, go for SYSTEM

	res = ReadINI (errStr, fname, "FONT SIZE", strRead);
	if (res>0 && sscanf(strRead.c_str(), "%d", &val)!=EOF && val >5)	fontsize = val;
	else																fontsize = DEFAULT_FONTSIZE;

	hdc = GetDC(NULL);
	lf.lfHeight = -MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
	eFont = CreateFont(lf.lfHeight,0,0,0, FW_MEDIUM, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		FIXED_PITCH | FF_MODERN, lf.lfFaceName);
    ::SendMessage(hAUXInput,WM_SETFONT,(WPARAM)eFont,1); 

	if (ReadINI (errStr, fname, "UDF_PATH", strRead)>=0)
		udfpath = strRead;
	else							
		udfpath="";

	if (ReadINI (errStr, fname, "AUX", strRead)>=0)
		m_auxstr = strRead; 
	else if (ReadINI (errStr, fname, "VAR", strRead)>=0)	{
		m_auxstr = strRead;
		if (ReadINI (errStr, fname, "SIGNAL", strRead)>=0) {
			m_auxstr += "\r\n" + strRead;
			printfINI (errStr, fname, "SIGNAL", "");
		}
		printfINI (errStr, fname, "VAR", "");
	} else {
		strcpy(estr, errStr);
		return 0;
	}
	SetDlgItemText(IDC_AUX, m_auxstr.c_str());
	return 1;
}

int CAuxDlg::writeINI(char *fname, char *estr)
{
	char errStr[256];
	char in[4096*16];
	if (!printfINI (errStr, fname, "SAMPLE RATE", "%d", fs)) {strcpy(estr, errStr); return 0;}
	if (!printfINI (errStr, fname, "FONT FACE", "%s", lf.lfFaceName)) {strcpy(estr, errStr); return 0;}
	if (!printfINI (errStr, fname, "FONT SIZE", "%d", fontsize)) {strcpy(estr, errStr); return 0;}
	if (!printfINI (errStr, fname, "UDF_PATH", "%s", udfpath.c_str())) {strcpy(estr, errStr); return 0;}
	if (!printfINI (errStr, fname, "PLAYBACK BLOCK SIZE MILLISEC", "%f", block)) {strcpy(estr, errStr); return 0;}
	GetDlgItemText(IDC_AUX, in, sizeof(in));
	if (!printfINI (errStr, fname, "AUX", "%s", in)) {strcpy(estr, errStr); return 0;}
	return 1;
}

void CAuxDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	char errstr[256], fname[MAX_PATH], fullfname[MAX_PATH];
	DWORD gle;
	CSignals dbclicked;
	char in[4096*16];
	string inn, sigstr;
	string parse[3]; 
	HDC hdc;
	switch (idc)
	{
	case IDC_MAKE_PLOT:
		GetDlgItemText(IDC_AUX, in, sizeof(in));
		m_auxstr = in;
		if (!PrepareSig())
			break;
		if (Sig.GetType() != CSIG_AUDIO) 
		{
			char buf[1000];
			switch(Sig.GetType())
			{
			case CSIG_EMPTY:
				break;
			case CSIG_SCALAR:
				sprintf(buf, "%f", Sig.value());
				break;
			case CSIG_STRING:
				strcpy(buf, "String \"");
				Sig.getString(buf+strlen(buf), sizeof(buf)/sizeof(*buf)-(int)strlen(buf));
				strcat(buf, "\"");
				break;
			case CSIG_VECTOR:
				int n = Sig.nSamples;
				sprintf(buf, "vector (%d)\n", n);
				for (int i=0; i<min(n,10); ++i)
					sprintf(buf+strlen(buf), "%f\n", Sig.buf[i]);
				if (n>10)
					strcat(buf, "...");
				break;
			}
			MessageBox(buf, "Result", MB_ICONINFORMATION);
			break;
		}
		Sig.MakeChainless();
		// get the last line into sigstr.
		for (size_t pp,p=strlen(in)+1; sigstr.length()==0; ) {
			pp = p;
			p=m_auxstr.rfind("\r\n", p-1);
			if (p == string::npos) {
				sigstr = m_auxstr.substr(0, pp);
				break;
			}
			sigstr = m_auxstr.substr(p+2, pp-p-2);
		}
		if (int len = Sig.nSamples)
		{
			RECT eer;
			GetWindowRect(&eer);
			CRect rt(0, 0, 500, 310);
//			RECT rt = {0, 0, 470, 280};
			HANDLE fig = OpenFigure(&rt, Sig, hDlg, 0, block);
			HANDLE ax = AddAxis (fig, .08, .17, .86, .8);
//			AddText(fig, sigstr.c_str(), .02, .02, 0, 0);
//			AddText(fig, in, -1, -1, 0, 0);
//			AddText(fig, "dummy", .09, .87, .86, .2);
			PlotCSignals(ax, Sig);
			if (Sig.next)
				PlotCSignals(ax, *Sig.next, RGB(200,0,50));
			hAccel = GetAccel(curFig = fig);
			hPlotDlgCurrent = GetHWND_PlotDlg(curFig);
			fig_hDlg[hPlotDlgCurrent] = curFig;
		}
		break;

	case ID_SETTINGS:
		hdc = GetDC(NULL);
		chf.hDC = CreateCompatibleDC( hdc );
		ReleaseDC( NULL, hdc );
		chf.lStructSize = sizeof(CHOOSEFONT);
		chf.hwndOwner = hDlg;
		chf.lpLogFont = &lf;
		chf.Flags = CF_ENABLEHOOK | CF_ENABLETEMPLATE | CF_INITTOLOGFONTSTRUCT;
		chf.rgbColors = RGB(0, 255, 255);
		chf.lCustData = (LPARAM)this;
		chf.hInstance = hInst;
		chf.lpszStyle = (LPTSTR)NULL;
		chf.nFontType = SCREEN_FONTTYPE;
		chf.nSizeMin = 0;
		chf.nSizeMax = 0;
		chf.lpfnHook = (LPCFHOOKPROC)SettingsHookProc;
		chf.lpTemplateName = "SETTINGS";
		ChooseFont( &chf );
		break;

	case ID_ABOUT:
		{
			CString msg, title = "AUX Viewer ";
			msg = title + VerStr;
			msg += "\n\nCopyright (C) 2010-2016 Bomjun Kwon, Ph.D.";
			msg += "\nUse this under Academic Free License v3.0";
			MessageBox(msg, "About " + title, MB_OK | MB_TOPMOST);
		}
		break;

	case IDCANCEL:
	case IDM_DESTROY:
		SendMessage (WM_CLOSE);
		break;
	case IDC_OPEN:
	case IDC_SAVE:
		fname[0]='\0';fullfname[0]='\0';
		if (!FileDlg(fdlg, fullfname, fname, idc))
		{
			gle = GetLastError();
			if (gle!=0)
			{
				sprintf(errstr, "GetOpenFileName failed. code=%d", gle);
				MessageBox (errstr, "AUX viewer", MB_OK);	
			}
		}
		else
		{
			SetDlgItemText(IDC_FNAME, fname);
			OnFile(fullfname, idc);
		}
		break;
	}
}

void CAuxDlg::OnFile(char *fname, int idc)
{
	char errstr[256];
	switch (idc)
	{
	case IDC_OPEN:
		if (!readINI(fname, errstr))
			MessageBox (errstr);
		break;
	case IDC_SAVE:
		if (!writeINI(fname, errstr))
			MessageBox (errstr);
		break;
	}
}


HBRUSH CAuxDlg::OnCtlColor(HDC hdc, HWND hCtrl, int id)
{
	COLORREF col2, col = RGB(190,230, 130);
	int idc;
	col2 = GetSysColor (COLOR_BTNFACE);
	idc = (int)::GetWindowLong (hCtrl, GWL_ID);
	switch (idc)
	{
	case IDC_AUX:
		SelectObject (hdc, eFont);
		SetBkColor (hdc, col);
		SetBkMode(hdc,OPAQUE);
		return hEditBrush;
		break;
	default:
		SetBkColor (hdc, col2);
	}
	return (HBRUSH) CreateSolidBrush (col2);
}

HBRUSH CAuxDlg::OnCtlColorStatic(HDC hdc, HWND hCtrl, int id)
{
	return OnCtlColor(hdc, hCtrl, id);
}

HBRUSH CAuxDlg::OnCtlColorEdit(HDC hdc, HWND hCtrl, int id)
{
	return OnCtlColor(hdc, hCtrl, id);
}



