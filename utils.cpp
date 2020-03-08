#pragma once
#include <vector>
#include <windows.h>
#include <Shlobj.h>
#include <cwctype> // iswspace
#include <algorithm> // remove_if
#include <map>
#include <sstream>

#define Assert(assertion) if(!(assertion))*(int*)NULL=0

//inline UINT ModifierToVK(UINT modifier) {
//	UINT vk=0;
//	if (modifier & MOD_ALT) vk = VK_MENU; //WTF?
//	else if (modifier & MOD_CONTROL) vk = VK_CONTROL;
//	else if (modifier & MOD_SHIFT) vk = VK_SHIFT;
//	//MOD_WIN is reserved for the OS, MOD_NOREPEAT is a behavior change
//	return vk;
//}

/// <summary>
/// Transforms hotkey modifiers of the form MOD_... to their virtual key counterpart VK_...
/// </summary>
/// <param name="modifiers">This can be any combination of MOD_CONTROL, MOD_ALT, MOD_SHIFT</param>
/// <returns>A vector of virtual keys for the corresponding modifiers</returns>
inline std::vector<UINT> SeparateModifiersToVK(UINT modifiers) {
	std::vector<UINT> mods;
	if (modifiers & MOD_CONTROL) mods.push_back(VK_CONTROL);
	if (modifiers & MOD_ALT) mods.push_back(VK_MENU); //WTF?
	if (modifiers & MOD_SHIFT) mods.push_back(VK_SHIFT);
	//MOD_WIN is reserved for the OS, MOD_NOREPEAT is a behavior change
	return mods;
}

/// <summary>
/// Transforms hotkey modifiers of the form MOD_... to their hotkey control modifier counterpart HOTKEYF_...
/// </summary>
/// <param name="modifiers">This can be any combination of MOD_CONTROL, MOD_ALT, MOD_SHIFT</param>
/// <returns></returns>
inline BYTE HotkeyModifiersToHotkeyControlModifiers(WORD modifiers) {
	BYTE hcm = 0;
	if (modifiers & MOD_ALT) hcm |= HOTKEYF_ALT;
	if (modifiers & MOD_CONTROL) hcm |= HOTKEYF_CONTROL;
	if (modifiers & MOD_SHIFT) hcm |= HOTKEYF_SHIFT;
	return hcm;
}

/// <summary>
/// Transforms hotkey control modifiers of the form HOTKEYF_... to their hotkey modifier counterpart MOD_...
/// </summary>
/// <param name="modifiers">This can be any combination of HOTKEYF_SHIFT, HOTKEYF_CONTROL, HOTKEYF_ALT</param>
/// <returns></returns>
inline WORD HotkeyControlModifiersToHotkeyModifiers(BYTE modifiers) {
	WORD hm = 0;
	if (modifiers & HOTKEYF_ALT) hm |= MOD_ALT;
	if (modifiers & HOTKEYF_CONTROL) hm |= MOD_CONTROL;
	if (modifiers & HOTKEYF_SHIFT) hm |= MOD_SHIFT;
	return hm;
}

/// <summary>
/// Generates the text form of the corresponding hotkey modifiers of the form MOD_..., and a virtual key
/// </summary>
/// <param name="modifiers">This can be any combination of MOD_CONTROL, MOD_ALT, MOD_SHIFT</param>
/// <param name="vk">Virtual key</param>
/// <returns></returns>
inline std::wstring HotkeyString(WORD modifiers, BYTE vk) {
	std::wstring hotkey_str = L"";
	BOOL first_mod = TRUE;
	if (modifiers & MOD_CONTROL) {
		if (!first_mod) hotkey_str += L"+";
		hotkey_str += L"Ctrl";
		first_mod = FALSE;
	}
	if (modifiers & MOD_ALT) {
		if (!first_mod) hotkey_str += L"+";
		hotkey_str += L"Alt";
		first_mod = FALSE;
	}
	if (modifiers & MOD_SHIFT) {
		if (!first_mod) hotkey_str += L"+";
		hotkey_str += L"Shift";
		first_mod = FALSE;
	}
	if (vk) {
		LONG scancode = ((LONG)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC) << 16);
		WCHAR vk_str[20];
		GetKeyNameTextW(scancode, vk_str, 20);
		if (*vk_str) {
			if (!first_mod) hotkey_str += L"+";
			hotkey_str += vk_str;
			first_mod = FALSE;
		}
	}
	return hotkey_str;
}

/// <summary>
/// Generates the text form of the corresponding hotkey control modifiers of the form HOTKEYF_..., and a virtual key
/// </summary>
/// <param name="modifiers">This can be any combination of HOTKEYF_SHIFT, HOTKEYF_CONTROL, HOTKEYF_ALT</param>
/// <param name="vk">Virtual key</param>
/// <returns></returns>
inline std::wstring ControlHotkeyString(BYTE modifiers, BYTE vk) {
	WORD mods = HotkeyControlModifiersToHotkeyModifiers(modifiers);
	return HotkeyString(mods, vk);
}

/// <summary>
/// See if a checkbox is checked
/// </summary>
/// <param name="checkbox"></param>
/// <returns>TRUE if it is checked, FALSE if it isn't</returns>
inline BOOL isChecked(HWND checkbox) {
	return ((SendMessageW(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE);
}

/// <summary>
/// Remove all whitespaces from a string
/// </summary>
/// <param name="text"></param>
inline void RemoveWhiteSpace(std::wstring &text) {
	text.erase(remove_if(text.begin(), text.end(), std::iswspace), text.end());
}

/// <summary>
/// Creates a map with Key-Value pairs
/// </summary>
/// <param name="s"></param>
/// <param name="separator">Character that separates the Key from the Value</param>
/// <returns></returns>
inline std::map<std::wstring, std::wstring> mappify(std::wstring const& s, wchar_t const& separator)
{
	std::map<std::wstring, std::wstring> m;

	std::wstring key, val;
	std::wistringstream iss(s);

	while (std::getline(std::getline(iss, key, separator) >> std::ws, val)) {
		RemoveWhiteSpace(key);
		RemoveWhiteSpace(val);//TODO(fran): it seems like the string to int/other functions already check for whitespace, do we take this one out?
		m[key] = val;
	}

	return m;
}

/// <summary>
/// Calculates the width of any RECT
/// </summary>
#define RECTWIDTH(r) (r.right >= r.left ? r.right - r.left : r.left - r.right )

/// <summary>
/// Calculates the height of any RECT
/// </summary>
#define RECTHEIGHT(r) (r.bottom >= r.top ? r.bottom - r.top : r.top - r.bottom )

/// <summary>
/// Check to see if animation has been disabled in Windows
/// </summary>
/// <returns></returns>
inline BOOL GetDoAnimateMinimize(VOID)
{//Thanks to: https://www.codeproject.com/Articles/735/Minimizing-windows-to-the-System-Tray
	ANIMATIONINFO ai;

	ai.cbSize = sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);

	return ai.iMinAnimate ? TRUE : FALSE;
}

// Returns the rect of where we think the system tray is. 
//If we can't find anything, we return a rect in the lower
//right hand corner of the screen
/// <summary>
/// Returns the rect of where we think the system tray is.
/// <para>If it can't find it, it returns a rect in the lower</para>
/// <para>right hand corner of the screen</para>
/// </summary>
/// <param name="lpTrayRect"></param>
inline void GetTrayWndRect(LPRECT lpTrayRect)
{//Thanks to: https://www.codeproject.com/Articles/735/Minimizing-windows-to-the-System-Tray
	// First, we'll use a quick hack method. We know that the taskbar is a window
	// of class Shell_TrayWnd, and the status tray is a child of this of class
	// TrayNotifyWnd. This provides us a window rect to minimize to. Note, however,
	// that this is not guaranteed to work on future versions of the shell. If we
	// use this method, make sure we have a backup!
	HWND hShellTrayWnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
	if (hShellTrayWnd)
	{
		HWND hTrayNotifyWnd = FindWindowEx(hShellTrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
		if (hTrayNotifyWnd)
		{
			GetWindowRect(hTrayNotifyWnd, lpTrayRect);
			return;
		}
	}

	// OK, we failed to get the rect from the quick hack. Either explorer isn't
	// running or it's a new version of the shell with the window class names
	// changed (how dare Microsoft change these undocumented class names!) So, we
	// try to find out what side of the screen the taskbar is connected to. We
	// know that the system tray is either on the right or the bottom of the
	// taskbar, so we can make a good guess at where to minimize to
	APPBARDATA appBarData;
	appBarData.cbSize = sizeof(appBarData);
	if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData))
	{
		// We know the edge the taskbar is connected to, so guess the rect of the
		// system tray. Use various fudge factor to make it look good
		switch (appBarData.uEdge)
		{
		case ABE_LEFT:
		case ABE_RIGHT:
			// We want to minimize to the bottom of the taskbar
			lpTrayRect->top = appBarData.rc.bottom - 100;
			lpTrayRect->bottom = appBarData.rc.bottom - 16;
			lpTrayRect->left = appBarData.rc.left;
			lpTrayRect->right = appBarData.rc.right;
			break;

		case ABE_TOP:
		case ABE_BOTTOM:
			// We want to minimize to the right of the taskbar
			lpTrayRect->top = appBarData.rc.top;
			lpTrayRect->bottom = appBarData.rc.bottom;
			lpTrayRect->left = appBarData.rc.right - 100;
			lpTrayRect->right = appBarData.rc.right - 16;
			break;
		}

		return;
	}

	// Blimey, we really aren't in luck. It's possible that a third party shell
	// is running instead of explorer. This shell might provide support for the
	// system tray, by providing a Shell_TrayWnd window (which receives the
	// messages for the icons) So, look for a Shell_TrayWnd window and work out
	// the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
	// and stretches either the width or the height of the screen. We can't rely
	// on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
	// rely on it being any size. The best we can do is just blindly use the
	// window rect, perhaps limiting the width and height to, say 150 square.
	// Note that if the 3rd party shell supports the same configuraion as
	// explorer (the icons hosted in NotifyTrayWnd, which is a child window of
	// Shell_TrayWnd), we would already have caught it above
	hShellTrayWnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);

#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

	if (hShellTrayWnd)
	{
		GetWindowRect(hShellTrayWnd, lpTrayRect);
		if (lpTrayRect->right - lpTrayRect->left > DEFAULT_RECT_WIDTH)
			lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
		if (lpTrayRect->bottom - lpTrayRect->top > DEFAULT_RECT_HEIGHT)
			lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;

		return;
	}

	// OK. Haven't found a thing. Provide a default rect based on the current work
	// area
	SystemParametersInfo(SPI_GETWORKAREA, 0, lpTrayRect, 0);
	lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
	lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;

#undef DEFAULT_RECT_WIDTH
#undef DEFAULT_RECT_HEIGHT
}

/// <summary>
/// Animates a window going to the tray and hides it
/// <para>Does not check whether the window is already minimized</para>
/// <para>The window position is left right where it started, so you can later use GetWindowRect to determine the "to" in TrayToWindow</para>
/// </summary>
/// <param name="from">Place from where the window will start moving, ej its top-left corner</param>
/// <param name="to">Window destination, aka the tray's top left corner, or wherever you want</param>
/// <param name="milliseconds">Duration of the animation</param>
/// <returns></returns>
inline void WindowToTray(HWND hWnd, POINT from, POINT to, int milliseconds) {
	double frametime = (1. / 120.)*1000.; //TODO(fran): this should be 2x monitor refresh rate
	float frames = milliseconds / frametime;
	RECT rc = { from.x,from.y,to.x,to.y };
	SIZE offset;
	offset.cx = RECTWIDTH(rc) / frames;
	offset.cy = RECTHEIGHT(rc) / frames;
	POINT sign;
	sign.x = (from.x >= to.x) ? -1 : 1;
	sign.y = (from.y >= to.y) ? -1 : 1;
	float alphaChange = 100 / frames;


	//AnimateWindow(hWnd, milliseconds, AW_BLEND | AW_HIDE); //Not even async support!

	// Set WS_EX_LAYERED on this window 
	SetWindowLong(hWnd,	GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	for (int i = 1; i <= frames; i++) {
		SetLayeredWindowAttributes(hWnd, NULL, (255 * (100 - alphaChange * i)) / 100, LWA_ALPHA);
		SetWindowPos(hWnd, NULL, from.x + sign.x*offset.cx*i, from.y + sign.y*offset.cy*i, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
		Sleep((int)floor(frametime));//good enough
	}
	SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
	SetWindowPos(hWnd, NULL, from.x, from.y, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
	ShowWindow(hWnd, SW_HIDE);
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) ^ WS_EX_LAYERED);

}

//TODO(fran): combine this two, or at least parts of them
/// <summary>
/// Animates a window coming out of the tray and shows it
/// </summary>
/// <param name="hWnd"></param>
/// <param name="from">Place from where the window will start moving, ej its top-left corner</param>
/// <param name="to">Window destination, aka the top left corner of where the window was before minimizing, or wherever you want</param>
/// <param name="milliseconds">Duration of the animation</param>
inline void TrayToWindow(HWND hWnd, POINT from, POINT to, int milliseconds) {
	double frametime = (1. / 120.)*1000.; //TODO(fran): this should be 2x monitor refresh rate
	float frames = milliseconds / frametime;
	RECT rc = { from.x,from.y,to.x,to.y };
	SIZE offset;
	offset.cx = RECTWIDTH(rc) / frames;
	offset.cy = RECTHEIGHT(rc) / frames;
	POINT sign;
	sign.x = (from.x >= to.x) ? -1 : 1;
	sign.y = (from.y >= to.y) ? -1 : 1;
	float alphaChange = 100 / frames;

	//We put the window at 0 alpha then we make the window visible 
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
	ShowWindow(hWnd, SW_SHOW);

	for (int i = 1; i <= frames; i++) {
		SetLayeredWindowAttributes(hWnd, NULL, (255 * (alphaChange * i)) / 100, LWA_ALPHA);
		SetWindowPos(hWnd, NULL, from.x + sign.x*offset.cx*i, from.y + sign.y*offset.cy*i, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
		Sleep((int)floor(frametime));//good enough
	}
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA); //just to make sure
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) ^ WS_EX_LAYERED);
	SetWindowPos(hWnd, NULL, to.x, to.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);//just to make sure, and we tell it to redraw this time

	SetActiveWindow(hWnd);
	SetForegroundWindow(hWnd);
}

/// <summary>
/// Minimizes a window and creates an animation to make it look like it goes to the tray
/// </summary>
/// <param name="hWnd"></param>
inline void MinimizeWndToTray(HWND hWnd)
{//Thanks to: https://www.codeproject.com/Articles/735/Minimizing-windows-to-the-System-Tray
	if (GetDoAnimateMinimize())
	{
		RECT rcFrom, rcTo;

		// Get the rect of the window. It is safe to use the rect of the whole
		// window - DrawAnimatedRects will only draw the caption
		GetWindowRect(hWnd, &rcFrom);
		GetTrayWndRect(&rcTo);

		WindowToTray(hWnd, { rcFrom.left,rcFrom.top }, { rcTo.left,rcTo.top }, 200);
	}
	else ShowWindow(hWnd, SW_HIDE);// Just hide the window
}

/// <summary>
/// Restores a window and makes it look like it comes out of the tray 
/// <para>and makes it back to where it was before minimizing</para>
/// </summary>
/// <param name="hWnd"></param>
inline void RestoreWndFromTray(HWND hWnd)
{//Thanks to: https://www.codeproject.com/Articles/735/Minimizing-windows-to-the-System-Tray
	if (GetDoAnimateMinimize())
	{
		// Get the rect of the tray and the window. Note that the window rect
		// is still valid even though the window is hidden
		RECT rcFrom, rcTo;
		GetTrayWndRect(&rcFrom);
		GetWindowRect(hWnd, &rcTo);

		// Get the system to draw our animation for us
		TrayToWindow(hWnd, { rcFrom.left,rcFrom.top }, { rcTo.left,rcTo.top }, 200);
	}
	else {
		// Show the window, and make sure we're the foreground window
		ShowWindow(hWnd, SW_SHOW);
		SetActiveWindow(hWnd);
		SetForegroundWindow(hWnd);
	}
}

/// <summary>
/// Get CPU frequency
/// </summary>
/// <returns>CPU frequency in milliseconds</returns>
inline double GetPCFrequency() {
	LARGE_INTEGER li;
	Assert(QueryPerformanceFrequency(&li));
	return double(li.QuadPart) / 1000.0; //milliseconds
}

/// <summary>
/// Sets the initial point of a counter that will later be used in GetCounter
/// <para>to determine the time passed between here and there</para>
/// </summary>
/// <param name="CounterStart"></param>
inline void StartCounter(__int64 &CounterStart) {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

/// <summary>
/// Get time passed since CounterStart, in the same unit of time as PCFreq
/// </summary>
/// <param name="CounterStart">Moment when the counter was started, use StartCounter</param>
/// <param name="PCFreq">CPU frequency of the PC, use GetPCFrequency, if this value is in milliseconds then so will be the returned value</param>
/// <returns>Time passed since CounterStart, in the time unit defined by PCFreq</returns>
inline double GetCounter(__int64 CounterStart, double PCFreq)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}