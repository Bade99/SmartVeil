#include "resource.h"

#include <Windows.h>
#include <string>
#include <Shlobj.h> //SHGetKnownFolderPath 
//#include <Windowsx.h> //GetStockBrush

#include "utils.cpp"
#include "Startup.cpp"
#include "Veil.h"
#include "Common.h"
#include "LANGUAGE_MANAGER.h"
#include "ControlProcedures.h"

//TODO(fran): this with the extern are ugly, look for better solution, maybe passing a bigger struct to the windows
CUSTOM_FRAME FRAME; //DEFINITION OF STATIC VARIABLE, so it's the same for all translation units (MUST be defined global)
_KNOWN_WINDOWS KNOWN_WINDOWS; //DEFINITION OF STATIC VARIABLE

//TODO(fran): implement resizer, probably with a system of columns, where each one contains rows
//	-----------------------------------------
//	|	|				|				|	|
//	|	|				|---------------|	|
//	|	|				|				|	|
//	|	|---------------|				|	|
//	|	|				|				|	|
//	|	|				|				|	|
//	-----------------------------------------

//TODO(fran): set lang at startup from WinMain instead of settings?

/// <summary>
/// Program entry point, lot's of setup happens here:
/// <para>-Checking no other instance of the program is running on this session</para>
/// <para>-Loading startup info</para>
/// <para>-Custom window classes</para>
/// <para>-Control colors setup</para>
/// <para>-Cleaning and saving at application end</para>
/// </summary>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ INT /*nShowCmd*/)
{
	const TCHAR* VeilClassName = L"Smart Veil Franco Badenas Abal";
	const TCHAR* VeilName = L"Smart Veil, the veil itself";

	//Check that no instance is already running on this user session
	//TODO(fran): from my tests this system is user session independent, check that's true for every case, 
	// eg. admin - admin, admin - normal user, normal user - normal user
	HANDLE single_instance_mutex = CreateMutex(NULL, TRUE, L"Smart Veil Single Instance Mutex Franco Badenas Abal");
	if (single_instance_mutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
		//If an instance already exists try to show its manager to the user
		//INFO: other ways of solving the hwnd finding: http://www.flounder.com/nomultiples.htm
		HWND existingApp = FindWindow(VeilClassName, VeilName);
		if (existingApp) PostMessage(existingApp, SCV_VEIL_SHOW_MGR, 0, 0);
		return 0; // Exit the app
	}
	defer{ ReleaseMutex(single_instance_mutex);	CloseHandle(single_instance_mutex); };

	// Load resources for class registration
	HCURSOR Cursor = LoadCursor(nullptr, IDC_ARROW);
	if (!Cursor)
	{
		ShowLastError(RCS(SCV_LANG_ERROR_CURSOR_LOAD), RCS(SCV_LANG_ERROR_SMARTVEIL));
		return 0;
	}
	defer{ DestroyCursor(Cursor); };

	HICON logo_icon, logo_icon_small;
	LoadIconMetric(hInstance, MAKEINTRESOURCE(LOGO_ICON), LIM_LARGE, &logo_icon);
	LoadIconMetric(hInstance, MAKEINTRESOURCE(LOGO_ICON), LIM_SMALL, &logo_icon_small);
	defer{ if (logo_icon) DestroyIcon(logo_icon); if (logo_icon_small) DestroyIcon(logo_icon_small); };

	//Define colors for controls
	ControlProcedures::Instance().Set_BackgroundColor(RGB(0, 0, 0));
	ControlProcedures::Instance().Set_HighlightColor(RGB(255, 255, 255));
	ControlProcedures::Instance().Set_PushColor(RGB(0, 110, 200));
	ControlProcedures::Instance().Set_MouseoverColor(RGB(0, 120, 215));
	ControlProcedures::Instance().Set_ComboIconID(COMBO_ICON);
	ControlProcedures::Instance().Set_CheckboxIconID(CHECKBOX_ICON);
	ControlProcedures::Instance().Set_CaptionCloseIconID(CROSS_ICON);
	ControlProcedures::Instance().Set_CaptionMinimizeIconID(MINIMIZE_ICON);

	// Register Veil class
	WNDCLASSEXW Wc;
	Wc.cbSize = sizeof(WNDCLASSEXW);
	Wc.style = CS_HREDRAW | CS_VREDRAW;
	Wc.lpfnWndProc = VeilProc;
	Wc.cbClsExtra = 0;
	Wc.cbWndExtra = 0;
	Wc.hInstance = hInstance;
	Wc.hIcon = logo_icon;
	Wc.hCursor = Cursor;
	Wc.hbrBackground = nullptr;
	Wc.lpszMenuName = nullptr;
	Wc.lpszClassName = VeilClassName;
	Wc.hIconSm = logo_icon_small;
	const ATOM veil_class = RegisterClassExW(&Wc);
	if (!veil_class)
	{
		ShowLastError(RCS(SCV_LANG_ERROR_WINDOWCLASS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL));
		return 0;
	}

	//Register Manager class
	WNDCLASSEXW WMc;
	WMc.cbSize = sizeof(WNDCLASSEXW);
	WMc.style = CS_HREDRAW | CS_VREDRAW;
	WMc.lpfnWndProc = MgrProc;
	WMc.cbClsExtra = 0;
	WMc.cbWndExtra = 0;
	WMc.hInstance = hInstance;
	WMc.hIcon = logo_icon;
	WMc.hCursor = Cursor;
	WMc.hbrBackground = ControlProcedures::Instance().Get_BackgroundBrush();
	WMc.lpszMenuName = nullptr;
	WMc.lpszClassName = L"Smart Veil Manager Franco Badenas Abal";
	WMc.hIconSm = logo_icon_small;
	const ATOM manager_class = RegisterClassExW(&WMc);
	if (!manager_class)
	{
		ShowLastError(RCS(SCV_LANG_ERROR_WINDOWCLASS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL));
		return 0;
	}

	//Register Settings class
	WNDCLASSEXW WSc;
	WSc.cbSize = sizeof(WNDCLASSEXW);
	WSc.style = CS_HREDRAW | CS_VREDRAW;
	WSc.lpfnWndProc = SettingsProc;
	WSc.cbClsExtra = 0;
	WSc.cbWndExtra = 0;
	WSc.hInstance = hInstance;
	WSc.hIcon = logo_icon;
	WSc.hCursor = Cursor;
	WSc.hbrBackground = ControlProcedures::Instance().Get_BackgroundBrush();
	WSc.lpszMenuName = nullptr;
	WSc.lpszClassName = L"Smart Veil Settings Franco Badenas Abal";
	WSc.hIconSm = logo_icon_small;
	const ATOM settings_class = RegisterClassExW(&WSc);
	if (!settings_class)
	{
		ShowLastError(RCS(SCV_LANG_ERROR_WINDOWCLASS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL)); //INFO TODO(fran): for the next project change naming convention for my strings, this is hard to read, for starters make them lower case
		return 0;
	}
	
	//Load startup info
	PWSTR folder_path;
	HRESULT folder_res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &folder_path);//TODO(fran): this is only supported on windows vista and above, will we ever care?
	//INFO: no retorna la ultima barra, la tenés que agregar vos, tambien te tenés que encargar de liberar el string
	Assert(SUCCEEDED(folder_res));

	STARTUP_INFO_PATH info_path; // Path to the file where we store our startup data
	info_path.known_folder = folder_path;
	CoTaskMemFree(folder_path);
	info_path.info_folder = L"Smart Veil";
	info_path.info_file = L"startup_info";
	info_path.info_extension = L"txt";//just so the user doesnt even have to open the file manually on windows

	STARTUP_INFO startup_info = read_startup_info_file(info_path.known_folder + L"\\" + info_path.info_folder + L"\\" + info_path.info_file + L"." + info_path.info_extension);

	// Create Veil window
	HWND veil_wnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		(LPCWSTR)MAKELONG(veil_class,0), VeilName,
		WS_POPUP,
		0, 0, 100, 100, //INFO: size and position don't matter since the veil is always maximized
		nullptr, nullptr, hInstance, nullptr);
	if (!veil_wnd)
	{
		ShowLastError(RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL));
		return 0;
	}
	KNOWN_WINDOWS.veil = veil_wnd;
	ShowWindow(veil_wnd, SW_MAXIMIZE);
	UpdateWindow(veil_wnd);


#define SETTINGS_NUMBER_OF_CHECKBOXES 30.f //This scales both mgr and settings windows
	const float CHECKBOX_SZ = GetSystemMetrics(SM_CXMENUCHECK)*.8f; //Size of one of our checkboxes that we use for determining window size
	SIZE mgr_wnd_sz; //TODO(fran): make this wnd more compact, and the same for its controls
	mgr_wnd_sz.cx = (LONG)(CHECKBOX_SZ * (SETTINGS_NUMBER_OF_CHECKBOXES*16.f / 9.f)); 
	mgr_wnd_sz.cy = (LONG)(mgr_wnd_sz.cx * 8.f / 16.f);
	//TODO(fran): when you create a window that is bigger than the screen in some axis, that axis gets automatically reduced, (we should check and resize-reposition accordingly)

	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);
	
#if 0 //test different sizes and aspect ratios
	{
		float WINDOW_REDUCTION = 4;
		mgr_wnd_sz.cx = 1024 / WINDOW_REDUCTION;//INFO(fran): remember now we are starting to use values given by windows for the specific screen, so things wont change even if the resolution does
		mgr_wnd_sz.cy = 768 / WINDOW_REDUCTION;
	}
#endif

	POINT mgr_wnd_pos;

	if (startup_info.settings.remember_manager_position 
		&& startup_info.manager.previous_screen_size.cx == GetSystemMetrics(SM_CXVIRTUALSCREEN)
		&& startup_info.manager.previous_screen_size.cy == GetSystemMetrics(SM_CYVIRTUALSCREEN))
		mgr_wnd_pos = { startup_info.manager.previous_manager_position.x, startup_info.manager.previous_manager_position.y };
	else
		mgr_wnd_pos = { (RECTWIDTH(desktopRect)) / 2 - mgr_wnd_sz.cx / 2 , (RECTHEIGHT(desktopRect)) / 2 - mgr_wnd_sz.cy / 2 };

	//Definition of custom frame (all in pixels)
	//SM_CXBORDER SM_CXFIXEDFRAME SM_CYFIXEDFRAME SM_CXEDGE SM_CYEDGE SM_CXPADDEDBORDER SM_CXSIZE SM_CXSIZEFRAME SM_CYSIZEFRAME SM_CXSMSIZE SM_CYCAPTION SM_CYSMCAPTION

	FRAME.caption_height = GetBasicWindowCaptionHeight(hInstance, mgr_wnd_pos);//INFO: I think this is dpi aware //TODO(fran): faster simpler method
	if (FRAME.caption_height <= 0) //INFO: menor igual me permite semi salvarla para multi-monitor //TODO(fran): is this neccessary???
		FRAME.caption_height = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER);//INFO: this is not dpi aware
	FRAME.bottom_border = FRAME.left_border = FRAME.right_border = 0;//We will have no borders, also we dont support resizing
	FRAME.caption_bk_brush = (HBRUSH)GetStockObject(DKGRAY_BRUSH); //TODO(fran): take this out of FRAME and just use the one in ControlProcedures
	FRAME.logo_icon = LOGO_ICON;
	FRAME.caption_font = CreateMyFont((LONG)(-FRAME.caption_height * .5));
	defer{ DeleteObject(FRAME.caption_font); }; //TODO(fran): should this be in the destructor for FRAME?
	FRAME.caption_text_color = ControlProcedures::Instance().Get_HighlightColor();

	//More Controls' color setup that should not be here
	{
		LOGBRUSH lb;
		GetObject(FRAME.caption_bk_brush, sizeof(LOGBRUSH), &lb);
		ControlProcedures::Instance().Set_CaptionBackgroundColor(lb.lbColor); //TODO(fran): will I use FRAME's or ControlProcedures' color?
	}

	//Create Manager window
	HWND mgr_wnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_APPWINDOW, (LPCWSTR)MAKELONG(manager_class,0), NULL,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MINIMIZEBOX ^ WS_MAXIMIZEBOX
		, mgr_wnd_pos.x, mgr_wnd_pos.y, mgr_wnd_sz.cx, mgr_wnd_sz.cy, veil_wnd, NULL, hInstance, &startup_info.manager);
	
	if (!mgr_wnd) {
		//TODO(fran): previously created windows should also be destroyed, how to do it? the problem with defer is it will also be called on normal program execution and we could destroy an HWND that has been assigned to someone else
		ShowLastError(RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL));
		return 0;
	}
	KNOWN_WINDOWS.mgr = mgr_wnd;
	AWT(mgr_wnd, SCV_LANG_MGR);

	UpdateWindow(mgr_wnd); //I do update first to semi-solve the ugly default ui problem
	
	if(startup_info.settings.show_manager_on_startup)
		ShowWindow(mgr_wnd, SW_SHOW);
	else
		ShowWindow(mgr_wnd, SW_HIDE);

	SIZE settings_wnd_sz;
	settings_wnd_sz.cx = (LONG)(CHECKBOX_SZ *SETTINGS_NUMBER_OF_CHECKBOXES);
	settings_wnd_sz.cy = (LONG)(settings_wnd_sz.cx * 11.f / 9.f);

	//Create Settings window
	HWND settings_wnd = CreateWindowEx(WS_EX_TOPMOST, (LPCWSTR)MAKELONG(settings_class,0), NULL,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MINIMIZEBOX ^ WS_MAXIMIZEBOX
		, mgr_wnd_pos.x, mgr_wnd_pos.y,
		settings_wnd_sz.cx, settings_wnd_sz.cy
		, mgr_wnd, NULL, hInstance, &startup_info.settings);

	if (!settings_wnd) {
		ShowLastError(RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL));
		//TODO(fran): all this errors should cleanup what's above, defer?
		return 0;
	}
	KNOWN_WINDOWS.settings = settings_wnd;
	AWT(settings_wnd, SCV_LANG_SETTINGS);
	ShowWindow(settings_wnd, SW_HIDE);
	UpdateWindow(settings_wnd);


	// Message loop
	MSG msg = { 0 };
	BOOL bRet;

	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1) { Assert(0);/*handle the error and possibly exit*/ }
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//Save startup_info //TODO(fran): should we save even if someone threw an exception/error in the loop?

	//TODO(fran): this is annoying, does the struct really need 4 strings?
	SaveStartupInfo(startup_info, info_path.known_folder + L"\\" + info_path.info_folder, info_path.info_file + L"." + info_path.info_extension);

	if (msg.message == WM_QUIT) // For a WM_QUIT message the wParam is the exit code
		return static_cast<INT>(msg.wParam);

	return 0;
}
