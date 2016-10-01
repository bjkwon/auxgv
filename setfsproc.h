#include <windows.h>
#include "WndDlg0.h" 

class CSetFS : public CWndDlg
{
public:
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnClose();
	void InitDialog2(int dummy);
	int fs;
	int fontsize;
	bool font_changed;
	LOGFONT      lf;
	LPCHOOSEFONT   lpchf;

	string udfpath;
	CSetFS();
	~CSetFS();
};
