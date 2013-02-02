/*
Copyright (C) 2001 DarkOne the Hacker
	http://darkone.yo.lv
	DarkOne@mail.navigators.lv

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "wcxhead.h"
#include "PACK.h"

/*
** Centers hWndDlg windows, rel. to hWndMain
*/
void CenterDialog(HWND hWndDlg, HWND hWndMain)
{
	RECT rect, Mainrect;
	
	if(GetWindowRect(hWndDlg, &rect) && GetWindowRect(hWndMain, &Mainrect))
	{
		SetWindowPos(hWndDlg, 0,
			Mainrect.left + ((Mainrect.right-Mainrect.left)-(rect.right-rect.left))/2,
			Mainrect.top + ((Mainrect.bottom-Mainrect.top)-(rect.bottom-rect.top))/2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
}

HINSTANCE hInstance;
HCURSOR hCursor;
/*
** About Dialog with one Command Button (Unload dialog on press)
** Center on Init rel. to main Window!
*/
BOOL CALLBACK AboutProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		CenterDialog(hWndDlg, GetParent(hWndDlg));
		hCursor=LoadCursor(NULL, IDC_HAND); // Windows 98/Me, Windows 2000/XP: Hand
		if(!hCursor)
			hCursor=LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MY_HAND));
		//SetClassLong(GetDlgItem(hWndDlg, IDC_STATIC_MAIL), GCL_HCURSOR, (LPARAM)hCursor);
		//SetClassLong(GetDlgItem(hWndDlg, IDC_STATIC_URL), GCL_HCURSOR, (LPARAM)hCursor);
		return TRUE;
	case WM_CTLCOLORSTATIC:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_STATIC_MAIL:
		case IDC_STATIC_URL:
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0x00, 0x00, 0xFF));
			return (BOOL)GetStockObject(NULL_BRUSH);
		case IDC_STATIC_DO:
			SetBkMode((HDC)wParam,TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0x00, 0xA0, 0x00));
			return (BOOL)GetStockObject(NULL_BRUSH);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			DestroyCursor(hCursor);
			EndDialog(hWndDlg, 1);
			return TRUE;
		case IDC_STATIC_URL:
			if(HIWORD(wParam)==STN_CLICKED)
			{
				ShellExecute(GetParent(hWndDlg), "open", "http://darkone.yo.lv", 0, 0, SW_SHOW);
				return TRUE;
			}
			break;
		case IDC_STATIC_MAIL:
			if(HIWORD(wParam)==STN_CLICKED)
			{
				ShellExecute(GetParent(hWndDlg), "open", "mailto:DarkOne@mail.navigators.lv?subject=PACK WinCmd Plugin", 0, 0, SW_SHOW);
				return TRUE;
			}
			break;
// 		case IDC_CHK_LCASE: FIXME
		}
		break;
	case WM_CLOSE:
		DestroyCursor(hCursor);
		EndDialog(hWndDlg, 0);
		return FALSE;
	}
	return FALSE;
}

void PAK_Configure(HWND Parent, HINSTANCE DllInstance)
{
	DialogBox(hInstance=DllInstance, MAKEINTRESOURCE(IDD_ABOUT), Parent, AboutProc);

}