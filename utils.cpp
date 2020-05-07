#pragma once
#include <windows.h>
#include <Shlobj.h> // HOTKEYF_ALT HOTKEYF_CONTROL HOTKEYF_SHIFT
#include <cwctype> // iswspace
#include <algorithm> // remove_if
#include <map>
#include <sstream>

//----------------------------------------------------------------------------------------------------------------------------------------------------
//This file defines useful functions and definitions that are for general use, not directly dependent or related to anything specified by this program
//----------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define Assert(assertion) if(!(assertion))*(int*)NULL=0
#else
#define Assert(assertion)  
#endif

inline std::wstring GetLastErrorAsString()
{
	//Thanks https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
	
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::wstring(); //No error message has been recorded

	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	std::wstring message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

/// <summary>
/// Returns a string with the format "HRESULT 0x########"
/// </summary>
/// <param name="h"></param>
/// <returns></returns>
inline std::wstring HRESULTToString(HRESULT h) { //TODO(fran): use wstring and avoid new and delete
	const UINT StringLen = sizeof(L"HRESULT 0x########") + 1;
	wchar_t* OutStr = new wchar_t[StringLen];
	if (!OutStr)
	{
		return std::wstring();
	}

	swprintf_s(OutStr, StringLen, L"HRESULT 0x%X", h);

	std::wstring hresult = OutStr;

	delete[] OutStr;

	return hresult;
}

/// <summary>
/// Creates MessageBox shodwing the last error as an understandable string + it's error code
/// </summary>
/// <param name="extra_info">Allows you to add a larger explanation of the problem</param>
/// <param name="title">MessageBox's title</param>
inline void ShowLastError(const std::wstring& extra_info, const std::wstring& title)
{
	const UINT flags = MB_OK | MB_TOPMOST;

	const std::wstring last_err = GetLastErrorAsString();

	const std::wstring hresult = HRESULTToString(HRESULT_FROM_WIN32(GetLastError()));

	std::wstring msg = extra_info!=L""? extra_info + L"\n" + last_err : last_err;
	msg += hresult;

	MessageBoxW(nullptr, msg.c_str(), title.c_str(), flags);
}

/// <summary>
/// Creates a Topmost MessageBox shodwing the error and the title
/// </summary>
inline void ShowError(const std::wstring& error, const std::wstring& title) {
	MessageBoxW(nullptr, error.c_str(), title.c_str(), MB_OK | MB_TOPMOST);
}

/// <summary>
/// Retrieve information from a Version resource
/// </summary>
/// <param name="hLib">The module from which to obtain the version info, for current thread use NULL</param>
/// <param name="versionID">The ID of the Version resource, aka MAKEINTRESOURCE(versionID)</param>
/// <param name="csEntry">
/// You can search for: 
/// "CompanyName"
/// "FileDescription" 
/// "FileVersion" 
/// "InternalName" 
/// "LegalCopyright" 
/// "OriginalFilename" 
/// "ProductName" 
/// "ProductVersion"
/// </param>
inline std::wstring GetVersionInfo(HMODULE hLib, UINT versionID, WCHAR* csEntry)
//Thanks https://www.codeproject.com/Articles/8628/Retrieving-version-information-from-your-local-app
//Code modified for wchar
{
	std::wstring csRet;

	HRSRC hVersion = FindResource(hLib, MAKEINTRESOURCE(versionID), RT_VERSION);
	if (hVersion)
	{
		HGLOBAL hGlobal = LoadResource(hLib, hVersion);
		if (hGlobal)
		{

			LPVOID versionInfo = LockResource(hGlobal);
			if (versionInfo)
			{
				DWORD vLen, langD;
				BOOL retVal;

				LPVOID retbuf = NULL;

				WCHAR fileEntry[256];

				swprintf(fileEntry, 256, L"\\VarFileInfo\\Translation");
				retVal = VerQueryValue(versionInfo, fileEntry, &retbuf, (UINT *)&vLen);
				if (retVal && vLen == 4)
				{
					memcpy(&langD, retbuf, 4);
					swprintf(fileEntry, 256, L"\\StringFileInfo\\%02X%02X%02X%02X\\%s",
						(langD & 0xff00) >> 8, langD & 0xff, (langD & 0xff000000) >> 24,
						(langD & 0xff0000) >> 16, csEntry);
				}
				else {
					swprintf(fileEntry, 256, L"\\StringFileInfo\\%04X04B0\\%s", GetUserDefaultLangID(), csEntry);
					//swprintf(fileEntry, 256, L"\\StringFileInfo\\040904b0\\%s", csEntry);
				}

				BOOL res = VerQueryValue(versionInfo, fileEntry, &retbuf, (UINT *)&vLen);
				if (res)
					csRet = (wchar_t*)retbuf;
				UnlockResource(hGlobal); //unnecessary
			}
			FreeResource(hGlobal);//unnecessary
		}
	}

	return csRet;

	//TODO(fran): instead of using the code above use this one which allows for multiple languages, we will need to check the ones it has
	// and see if any corresponds with the current lang in LANGUAGE_MANAGER, if not look for english
	/*
	HRESULT hr;

	// Structure used to store enumerated languages and code pages.
	struct LANGANDCODEPAGE {
	  WORD wLanguage;
	  WORD wCodePage;
	} *lpTranslate;

	// Read the list of languages and code pages.

	VerQueryValue(pBlock,
				  TEXT("\\VarFileInfo\\Translation"),
				  (LPVOID*)&lpTranslate,
				  &cbTranslate);

	// Read the file description for each language and code page.

	for( i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
	{
	  hr = StringCchPrintf(SubBlock, 50,
				TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),
				lpTranslate[i].wLanguage,
				lpTranslate[i].wCodePage);
		if (FAILED(hr))
		{
		// TODO: write error handler.
		}

	  // Retrieve file description for language and code page "i".
	  VerQueryValue(pBlock,
					SubBlock,
					&lpBuffer,
					&dwBytes);
	}
	*/
}

/// <summary>
/// Transforms hotkey modifiers of the form MOD_... to their virtual key counterpart VK_...
/// </summary>
/// <param name="modifiers">This can be any combination of MOD_CONTROL, MOD_ALT, MOD_SHIFT</param>
/// <returns>A vector of virtual keys for the corresponding modifiers</returns>
//inline std::vector<UINT> SeparateModifiersToVK(UINT modifiers) {
//	std::vector<UINT> mods;
//	if (modifiers & MOD_CONTROL) mods.push_back(VK_CONTROL);
//	if (modifiers & MOD_ALT) mods.push_back(VK_MENU); //WTF?
//	if (modifiers & MOD_SHIFT) mods.push_back(VK_SHIFT);
//	//MOD_WIN is reserved for the OS, MOD_NOREPEAT is a behavior change
//	return mods;
//}

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
/// <para>If any whitespace is found it is removed</para>
/// </summary>
/// <param name="s"></param>
/// <param name="separator">Character that separates the Key from the Value</param>
/// <returns></returns>
inline std::map<const std::wstring, std::wstring> mappify(const WCHAR* s, wchar_t separator)
{
	std::map<const std::wstring, std::wstring> m;

	std::wstring key, val;
	std::wistringstream iss(s);

	while (std::getline(std::getline(iss, key, separator) >> std::ws, val)) {
		RemoveWhiteSpace(key);
		RemoveWhiteSpace(val);
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
	float frametime = (1.f / 120.f)*1000.f; //TODO(fran): this should be 2x monitor refresh rate
	float frames = milliseconds / frametime;
	RECT rc = { from.x,from.y,to.x,to.y };
	SIZE offset;
	offset.cx = (LONG)(RECTWIDTH(rc) / frames);
	offset.cy = (LONG)(RECTHEIGHT(rc) / frames);
	POINT sign;
	sign.x = (from.x >= to.x) ? -1 : 1;
	sign.y = (from.y >= to.y) ? -1 : 1;
	float alphaChange = 100 / frames;


	//AnimateWindow(hWnd, milliseconds, AW_BLEND | AW_HIDE); //Not even async support!

	// Set WS_EX_LAYERED on this window 
	SetWindowLong(hWnd,	GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	for (int i = 1; i <= frames; i++) {
		SetLayeredWindowAttributes(hWnd, NULL, (BYTE)((255 * (100 - alphaChange * i)) / 100), LWA_ALPHA);
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
	float frametime = (1.f / 120.f)*1000.f; //TODO(fran): this should be 2x monitor refresh rate
	float frames = milliseconds / frametime;
	RECT rc = { from.x,from.y,to.x,to.y };
	SIZE offset;
	offset.cx = (LONG)(RECTWIDTH(rc) / frames);
	offset.cy = (LONG)(RECTHEIGHT(rc) / frames);
	POINT sign;
	sign.x = (from.x >= to.x) ? -1 : 1;
	sign.y = (from.y >= to.y) ? -1 : 1;
	float alphaChange = 100 / frames;

	//We put the window at 0 alpha then we make the window visible 
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
	ShowWindow(hWnd, SW_SHOW);

	for (int i = 1; i <= frames; i++) {
		SetLayeredWindowAttributes(hWnd, NULL, (BYTE)((255 * (alphaChange * i)) / 100), LWA_ALPHA);
		SetWindowPos(hWnd, NULL, from.x + sign.x*offset.cx*i, from.y + sign.y*offset.cy*i, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);
		Sleep((int)floor(frametime));//good enough
	}
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA); //just to make sure
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) ^ WS_EX_LAYERED);
	SetWindowPos(hWnd, NULL, to.x, to.y, 0, 0, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOSIZE);//just to make sure, dont tell it to redraw cause it takes a long time to do it and looks bad

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

/// <summary>
/// Get color of an HBRUSH
/// </summary>
/// <param name="br"></param>
/// <returns></returns>
inline COLORREF ColorFromBrush(HBRUSH br) {
	LOGBRUSH lb;
	GetObject(br, sizeof(lb), &lb);
	return lb.lbColor;
}

/// <summary>
/// Returns full path to the program's exe, or L"" on failure
/// </summary>
inline std::wstring GetExePath() {
	WCHAR exe_path[MAX_PATH];
	GetModuleFileNameW(NULL, exe_path, MAX_PATH);//INFO: last param is size of buffer in TCHAR
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		//path is too long for the buffer, it must be using the \\?\ prefix
		WCHAR* long_exe_path = new (std::nothrow) WCHAR(32767);//INFO: documentation says this max value is approximate
		if (!long_exe_path) return L"";
		GetModuleFileNameW(NULL, long_exe_path, 32767);//TODO(fran): check whether this works or needs something extra
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) { delete[] long_exe_path; return L""; }
		std::wstring long_path = long_exe_path;
		delete[] long_exe_path;
		return long_exe_path;
	}
	return exe_path;
}


/// <summary>
/// Simple hack to retrieve the size of a normal window's title area
/// </summary>
/// <param name="aprox_pos">Position(top-left corner) where you'll place your real window</param>
/// <returns>Height of the title/caption area</returns>
inline int GetBasicWindowCaptionHeight(HINSTANCE hInstance, POINT aprox_pos) {//TODO(fran): make this work for multi-monitor
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {return DefWindowProc(hwnd, msg, wParam, lParam); };
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Franco Test Class Height";
	wcex.hIconSm = NULL;

	ATOM reg_res = RegisterClassExW(&wcex);

	if (!reg_res) {
		return 0;
	}

	HWND hWnd = CreateWindowExW(NULL, L"Franco Test Class Height", L"Nothing", WS_OVERLAPPEDWINDOW
		, aprox_pos.x, aprox_pos.y, 200, 200, nullptr, nullptr, hInstance, nullptr);
	//INFO: for position use the same as the manager
	//INFO: the 200, 200 is equal to getwindowrect

	if (!hWnd)
	{
		return 0;
	}

	//ShowWindow(hWnd, nCmdShow);
	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	RECT rw, rc;
	GetClientRect(hWnd, &rc);
	GetWindowRect(hWnd, &rw);

	//First see what the sizing border is for the bottom part

	int border = (int)((RECTWIDTH(rw) - RECTWIDTH(rc)) / 2.f);

	int caption_height = RECTHEIGHT(rw) - RECTHEIGHT(rc) - border;

	DestroyWindow(hWnd);
	UnregisterClass(L"Franco Test Class Height", hInstance);

	return caption_height;
}

/// <summary>
/// Extract the first number that appears in a string
/// </summary>
/// <param name="str">The string that contains a number somewhere in its contents</param>
/// <returns>The number as a string</returns>
inline std::wstring first_number_in_string(std::wstring const & str)
{
//Thanks to https://stackoverflow.com/questions/30073839/c-extract-number-from-the-middle-of-a-string
	std::size_t const n = str.find_first_of(L"0123456789");
	if (n != std::wstring::npos)
	{
		std::size_t const m = str.find_first_not_of(L"0123456789", n);
		return str.substr(n, m != std::string::npos ? m - n : m);
	}
	return std::wstring();
}

//inline UINT ModifierToVK(UINT modifier) {
//	UINT vk=0;
//	if (modifier & MOD_ALT) vk = VK_MENU; //WTF?
//	else if (modifier & MOD_CONTROL) vk = VK_CONTROL;
//	else if (modifier & MOD_SHIFT) vk = VK_SHIFT;
//	//MOD_WIN is reserved for the OS, MOD_NOREPEAT is a behavior change
//	return vk;
//}