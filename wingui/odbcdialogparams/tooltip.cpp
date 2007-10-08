#include "stdafx.h"
#include "resource.h"

// DoCreateDialogTooltip - creates a tooltip control for a dialog box,  
//     enumerates the child control windows, and installs a hook 
//     procedure to monitor the message stream for mouse messages posted 
//     to the control windows. 
// Returns TRUE if successful or FALSE otherwise. 
// 
// Global variables 
// g_hinst - handle of the application instance 
// g_hwndTT - handle of the tooltip control 
// g_hwndDlg - handle of the dialog box 

// g_hhk - handle of the hook procedure 
HHOOK g_hhk;
HWND g_hwndTT;

extern HINSTANCE ghInstance;
#define g_hinst ghInstance

extern HWND g_hwndDlg;

BOOL CALLBACK EnumChildProc(HWND hwndCtrl, LPARAM lParam) ;
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) ;

BOOL DoCreateDialogTooltip(void) 
{ 
 
    // Ensure that the common control DLL is loaded, and create 
    // a tooltip control. 
    InitCommonControls(); 
    g_hwndTT = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, 
        TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        CW_USEDEFAULT, g_hwndDlg, NULL, g_hinst, NULL); 
 
    if (g_hwndTT == NULL) 
        return FALSE; 
 
    // Enumerate the child windows to register them with the tooltip
    // control. 

    if (!EnumChildWindows(g_hwndDlg, (WNDENUMPROC) EnumChildProc, 0)) 
        return FALSE; 
 
    // Install a hook procedure to monitor the message stream for mouse 
    // messages intended for the controls in the dialog box. 
    g_hhk = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,     
        (HINSTANCE) NULL, GetCurrentThreadId()); 
 
    if (g_hhk == (HHOOK) NULL) 
        return FALSE; 
 
    return TRUE; 
} 
 
// EmumChildProc - registers control windows with a tooltip control by
//     using the TTM_ADDTOOL message to pass the address of a 

//     TOOLINFO structure. 
// Returns TRUE if successful or FALSE otherwise. 
// hwndCtrl - handle of a control window 
// lParam - application-defined value (not used) 
BOOL CALLBACK EnumChildProc(HWND hwndCtrl, LPARAM lParam) 
{ 
    TOOLINFO ti; 
    wchar_t szClass[64]; 
 
    // Skip static controls. 
    GetClassName(hwndCtrl, szClass, sizeof(szClass)); 
    if (!lstrcmp(szClass, L"Edit")) { 
        ti.cbSize = sizeof(TOOLINFO); 
        ti.uFlags = TTF_IDISHWND; 

        ti.hwnd = g_hwndDlg; 
        ti.uId = (UINT) hwndCtrl; 
        ti.hinst = g_hinst; 
        ti.lpszText = LPSTR_TEXTCALLBACK; 
        int res = SendMessage(g_hwndTT, TTM_ADDTOOL, 0, 
            (LPARAM) (LPTOOLINFO) &ti); 
		int e = GetLastError();
		res++;
    } 
    return TRUE; 
} 
 
// GetMsgProc - monitors the message stream for mouse messages intended 
//     for a control window in the dialog box. 
// Returns a message-dependent value. 
// nCode - hook code 
// wParam - message flag (not used) 

// lParam - address of an MSG structure 
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
    MSG *lpmsg; 
 
    lpmsg = (MSG *) lParam; 
    if (nCode < 0 || !(IsChild(g_hwndDlg, lpmsg->hwnd))) 
        return (CallNextHookEx(g_hhk, nCode, wParam, lParam)); 
 
    switch (lpmsg->message) { 
        case WM_MOUSEMOVE: 
        case WM_LBUTTONDOWN: 
        case WM_LBUTTONUP: 
        case WM_RBUTTONDOWN: 
        case WM_RBUTTONUP: 
            if (g_hwndTT != NULL) { 

                MSG msg; 
 
                msg.lParam = lpmsg->lParam; 
                msg.wParam = lpmsg->wParam; 
                msg.message = lpmsg->message; 
                msg.hwnd = lpmsg->hwnd;
                SendMessage(g_hwndTT, TTM_RELAYEVENT, 0, 
                    (LPARAM) (LPMSG) &msg); 
            } 
            break; 
        default: 
            break; 
    } 
    return (CallNextHookEx(g_hhk, nCode, wParam, lParam)); 
} 
 
// OnWMNotify - provides the tooltip control with the appropriate text 

//     to display for a control window. This function is called by 
//     the dialog box procedure in response to a WM_NOTIFY message. 
// lParam - second message parameter of the WM_NOTIFY message 
VOID OnWMNotify(WPARAM wParam, LPARAM lParam) 
{ 
    LPTOOLTIPTEXT lpttt; 
    int idCtrl; 
 
    if ((((LPNMHDR) lParam)->code) == TTN_NEEDTEXT) { 
        idCtrl = GetDlgCtrlID((HWND) ((LPNMHDR) lParam)->idFrom); 
        lpttt = (LPTOOLTIPTEXT) lParam; 
 
        switch (idCtrl) { 
#define SET_TIP(edit,tip)\
			case IDC_EDIT_##edit: \
                lpttt->lpszText = tip; \
                return; 

			SET_TIP(drvname, L"An unique name for this data source");
			SET_TIP(drvdesc, L"A brief description for this data source");
			SET_TIP(srvname, L"The hostname for the MySQL Server");
			SET_TIP(port, L"The TCP/IP port to use if server is not localhost");
			SET_TIP(username, L"The username used to connect to MySQL");
			SET_TIP(password, L"The password for the server user combination");
			SET_TIP(dbname, L"The database to be current upon connect");
        } 
    } 
return; 
} 
