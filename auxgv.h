#pragma once

#include "WndDlg0.h"
#include <commctrl.h>  // includes status bar
#include <vector>  
#include "resource.h"
#include "msgCrack_seacrm.h"
#include "audfret.h"
#include "audstr.h"
#include "sigproc.h"
#include "graffy.h"

#define INIFILE "auxgv.ini"
#define DEFAULT_FS 22050
#define DEFAULT_FONTSIZE 20
#define DEFAULT_BLOCK_SIZE 200.

#define WM__FONTCHANGED	WM_APP+111

BOOL CALLBACK  SettingsHookProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
