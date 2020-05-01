#include "Manager.h"

#include "resource.h"

#include <string>
#include <commctrl.h>
#include <uxtheme.h>// setwindowtheme
#include <dwmapi.h>
#include <Windowsx.h>
//#include <vssym32.h>

#include "DisplayManager.h"
#include "DuplicationManager.h"

#include "ImagePresentation.h"

#include "Common.h"
#include "utils.cpp"
#include "fmt/format.h" //Thanks to https://github.com/fmtlib/fmt
#include "TOOLTIP_REPO.h"
#include "TRAY_HANDLER.h"
#include "LANGUAGE_MANAGER.h"
#include "ControlProcedures.h"


#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"UxTheme.lib")// setwindowtheme
#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"Version.lib") //for VerQueryValue

//Definition of child control ids and internal messages
#define SCV_MANAGER_THRESHOLD_TITLE (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+1) //The static text control that shows the threshold slider's current value
#define SCV_MANAGER_OPACITY_TITLE (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+2) //The static text control that shows the opacity slider's current value
#define SCV_MANAGER_SECRET_ABOUT (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+3) //Button for showing about information
#define SCV_MANAGER_SETTINGS (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+4) //Button to launch the settings
#define SCV_MANAGER_LANG_DYNAMIC_UPDATE (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+5) //Identifier for the LanguageManager to call the manager when dynamic windows (ones where the text constantly changes) need text updates
#define SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+6) //Msg sent by the manager to itself to update the turn on-off button when it's pressed and on language change
#define SCV_MANAGER_UPDATE_THRESHOLD_OPACITY (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+7) //Msg sent by the manager to itself to update the sliders' text when there's a language change
#define SCV_MANAGER_TOOLTIP_THRESHOLD_SLIDER (SCV_MANAGER_FIRST_INTERNAL_MESSAGE+8) //The Slider control for the threshold




//TODOs:
//CREATE NAMESPACES
//·Icons on buttons are not correctly centered
//·Create one big square for the tooltips of threshold and opacity each one encompassing the area from the beginning of the text to the end of the slider?
//·When sliders are pressed there is a white border around it, why?, also the thingy that moves the slider is now white when moved, why? looks better though
//·When the veil gets turned on it starts slowly taking mem to around 11.5MB and then stops, what is happening there?
//·When no language was found on settings file use the system default if we support it, otherwise english
//·Need to manage erasebkgrnd on buttonproc, getting white backgrounds some times, probably there's some other bug too
//·From what I read basic tooltips have an 80 character limit, what do we do? other langs could go over that
//·We are setting the lang on per thread basis, what happens when thread from outmgr,threadmgr,dispmgr requests a string??
//·Compile 64 bit ? 
//·Change mouse icon to a hand for controls that arent entirely obvious the user can click on or use them
//·Allow the user to decide the update rate of the veil, also useful to know it for testing
//·Button press must only be accepted if when released mouse is inside button area, also we are doing the same thing wrong when we draw
//·Sliders should go to 100%
// and always put a % after the number
//?·Add an information icon (that little circle with a i) next to threshold, so the user can click and better understand?
//·Give the user the option to write the threshold/opacity value, if they click over the number ?
//·Copyright? I want it open source though
//·Painting the hotkey green and red isnt color blind helpful, ideas? - I could show a tooltip that says it failed or succeeded
//·A simpler name instead of threshold for the text control
//·Automatic opacity increase in a time interval? eg from 0:00 to 4:00
//·Settings: save button greyed out until something is changed
//·Find out why just the mouse movement is causing the highest of cpu and gpu usage, look into the other cpp files
//·IMPORTANT: the fact that we update our veil means there is a new frame that windows has to present, therefore we are always generating more
// and more frames non stop, can we fix this somehow? ie skip one frame update each time
//LOOKs:
//·Make minimize animation go towards tray when tray icon is enabled
//·When the user changes to another window we could slightly change the color to indicate that we are not the focused window anymore
//·I like how the settings icon looks without the white border, we could also make light-blue only inside the icon
//·Make the icon have a hidden design that can only be viewed in the tray when the threshold & opacity are at some point x
//·Change mouse icon when manager is open, to one that only has white borders, and the inside transparent or black?
//·Application icon, and assign it to both the veil and the manager, to the settings I dont know
//·Flip the combobox button icon when displaying the list

//INFO:
//
//·Buddy windows: links controls together! could try to use it to auto update position on lang change, dont yet know if they have update properties
//·Paint title bar: http://www.it-quants.com/Blogs/tabid/83/EntryId/53/Win32-SDK-how-to-change-the-title-bar-color-title.aspx
//·Custom draw?: https://docs.microsoft.com/en-us/windows/win32/controls/custom-draw
//·Custom frame, looks like guaranteed suffering: https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
//·How to add icons to visual studio: create default icon, change name to your icon's, copy your icon in the same folder, done! thanks vs

// )appdata roaming( , in a way I like those inverse parenthesis

//THANKs:
//·Micaela Zabatta (1st tester)

/// <summary>
/// Full control setup for the Manager window
/// </summary>
/// <param name="hWnd"></param>
/// <param name="hInstance"></param>
/// <param name="startup_info"></param>
void SetupMgr(HWND hWnd,HINSTANCE hInstance,const CUSTOM_FRAME& frame, const MANAGER& manager_data,HFONT& manager_font) {
	RECT windowRect;
	GetMyClientRect(hWnd, frame, &windowRect);
	
	float width = (float)RECTWIDTH(windowRect);
	float height = (float)RECTHEIGHT(windowRect);

	float paddingX = windowRect.left + width * .1f; //at .1 objects will start 10% to the side of the window
	float paddingY = height*.114f; //at .1 there will be less than 10 objects for sure
	float addPaddingY = paddingY;
	paddingY += windowRect.top;

	float SecondColumnX = width*.51f + windowRect.left;

	float ThresholdTextX = width * .335f;
	float TextY = height * .14f;
	HWND ThresholdText = CreateWindowW(L"Static",NULL, WS_VISIBLE | WS_CHILD | SS_NOTIFY | SS_CENTERIMAGE | SS_CENTER
		, (int)paddingX, (int)paddingY, (int)ThresholdTextX, (int)TextY,hWnd,(HMENU)SCV_MANAGER_THRESHOLD_TITLE,NULL,NULL);

	//TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, ThresholdText, SCV_LANG_MGR_THRESHOLD_TIP);
	TOOLTIP_REPO::Instance().CreateToolTip(ThresholdText, SCV_LANG_MGR_THRESHOLD_TIP);

	paddingY += TextY *1.6f;

	float SliderX = ThresholdTextX;
	float SliderY = height * .091f;
	HWND ThresholdSlider = CreateWindowExW(0, TRACKBAR_CLASS, 0, WS_CHILD| WS_VISIBLE | TBS_NOTICKS
		, (int)paddingX, (int)paddingY, (int)SliderX, (int)SliderY, hWnd, (HMENU)SCV_MANAGER_TOOLTIP_THRESHOLD_SLIDER, NULL, NULL);

	paddingY += addPaddingY+ SliderY;

	TOOLTIP_REPO::Instance().CreateToolTip(ThresholdSlider, SCV_LANG_MGR_THRESHOLD_TIP);

	SendMessage(ThresholdSlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 99));
	SendMessage(ThresholdSlider, TBM_SETPAGESIZE, 0, (LPARAM)5);
	SendMessage(ThresholdSlider, TBM_SETLINESIZE, 0, (LPARAM)5);
	//TODO(fran): change the amount the trackbar moves when the mousewheel is scrolled
	//·https://www.purebasic.fr/english/viewtopic.php?t=38749
	SendMessage(ThresholdSlider, TBM_SETPOS,(WPARAM)TRUE, (LPARAM)manager_data.slider_threshold_pos);


	//float OpacityTextX = width * .165f;
	HWND OpacityText = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_NOTIFY | SS_CENTERIMAGE | SS_CENTER
		, (int)paddingX, (int)paddingY, (int)ThresholdTextX, (int)TextY, hWnd, (HMENU)SCV_MANAGER_OPACITY_TITLE, NULL, NULL);

	TOOLTIP_REPO::Instance().CreateToolTip(OpacityText, SCV_LANG_MGR_OPACITY_TIP);

	paddingY += TextY * 1.6f;

	HWND OpacitySlider = CreateWindowExW(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_NOTICKS
		, (int)paddingX, (int)paddingY, (int)SliderX, (int)SliderY, hWnd, (HMENU)SCV_MANAGER_TOOLTIP_OPACITY_SLIDER, NULL, NULL);

	TOOLTIP_REPO::Instance().CreateToolTip(OpacitySlider, SCV_LANG_MGR_OPACITY_TIP);

	paddingY += addPaddingY + SliderY;

	SendMessage(OpacitySlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 99));
	SendMessage(OpacitySlider, TBM_SETPAGESIZE, 0, (LPARAM)5);
	SendMessage(OpacitySlider, TBM_SETLINESIZE, 0, (LPARAM)5);
	SendMessage(OpacitySlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)manager_data.slider_opacity_pos);

	//A bit of a hack to pipe the necessary messages for setting up OutMgr and the slider's text
	SendMessage(hWnd, SCV_MANAGER_UPDATE_THRESHOLD_OPACITY, 0, 0);

	//Restart paddingY to configure second column
	paddingY = addPaddingY + windowRect.top;

	//INITCOMMONCONTROLSEX comctl;
	//comctl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//comctl.dwICC = ICC_STANDARD_CLASSES;
	//InitCommonControlsEx(&comctl);

	float SettingX = width * .33f;
	float SettingY = height * .205f;
	HWND TurnOnOff = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW //| BS_NOTIFY
		, (int)SecondColumnX, (int)paddingY, (int)SettingX, (int)SettingY, hWnd, (HMENU)SCV_MANAGER_TURN_ON_OFF, NULL, NULL);
	SetWindowSubclass(TurnOnOff, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	paddingY += addPaddingY + SettingY;

	//Font set-up
	manager_font = CreateMyFont((LONG)(-width * .046f));

	if (manager_font == NULL)
	{
		MessageBox(hWnd, RCS(SCV_LANG_ERROR_FONT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), MB_OK);
	}
	else {
		EnumChildWindows(hWnd, [](HWND child, LPARAM font) ->BOOL {SendMessageW(child, WM_SETFONT, (WPARAM)(HFONT)font, TRUE); return TRUE; }
		, (LPARAM)manager_font);
	}

	float secretX = width*.1f;
	float secretY = height*.1f;
	HWND secret_button = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW
		, windowRect.left, windowRect.top + (int)(height-secretY), (int)secretX, (int)secretY, hWnd, (HMENU)SCV_MANAGER_SECRET_ABOUT, NULL, NULL);

	SetWindowSubclass(secret_button, ControlProcedures::Instance().SecretButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//Settings button control
	//TODO(fran): decide proper placement and size
	POINT settings_pos;
	POINT settings_size; 
	settings_size.x = (LONG)(height * .137f); 
	settings_size.y = (LONG)(settings_size.x);
	//settings_size.x+=2; //INFO: Trying to make the button be less of a square so it is easier for the icon to be correctly centered
	//if ((settings_size.x % 2) != 0) settings_size.x+=1;
	float settings_separation = height*.1f;
	settings_pos.x = (int)(windowRect.left + width - settings_size.x - settings_separation);
	settings_pos.y = (int)(windowRect.top + height - settings_size.y - settings_separation);
	HWND Settings = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_ICON //| BS_OWNERDRAW
		, settings_pos.x, settings_pos.y, settings_size.x, settings_size.y, hWnd, (HMENU)SCV_MANAGER_SETTINGS, hInstance, NULL);

	SetWindowSubclass(Settings, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//INFO IMPORTANT: every button that needs and icon drawn will send the icon value through the BM_SETIMAGE message
	//SetWindowLongPtr(Settings, GWL_USERDATA, SETTINGS_ICON);
	SendMessage(Settings, BM_SETIMAGE, IMAGE_ICON, SETTINGS_ICON);
	
	TOOLTIP_REPO::Instance().CreateToolTip(Settings, SCV_LANG_MGR_SETTINGS);

}

/// <summary>
/// Used to notify on startup errors that need to close the program
/// </summary>
inline void ShowClosingAppError(UINT error_stringID) {
	std::wstring error_msg = RS(error_stringID);
	error_msg += L" " + std::to_wstring(GetLastError()) + L"\n" + RS(SCV_LANG_ERROR_CLOSING_APP);
	MessageBoxW(NULL, error_msg.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), MB_OK | MB_TOPMOST);
}

struct gcbi { HWND child; int ID; };
inline HWND get_child_by_id(HWND parent, int ID) { //TODO(fran): this is probably very inefficient, we need to store the guys we want in a struct, or something like that
	gcbi params{ NULL,ID };
	EnumChildWindows(parent, [](HWND child, LPARAM _gcbi) ->BOOL {
		if (GetDlgCtrlID(child) == ((gcbi*)_gcbi)->ID) 
			{ ((gcbi*)_gcbi)->child = child; return FALSE; }
		return TRUE; 
	}
	, (LPARAM)&params);
	return params.child;
}



LRESULT CALLBACK MgrProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static WORKER_THREAD_INIT thread_data;
	static MANAGER* mgr_data;
	static HANDLE worker_thread_handle;
	static HFONT manager_font;

	switch (message)
	{
	case WM_CREATE:
	{
		//Setup all the controls and the "look and feel" of the window
		SetWindowTheme(hWnd, L"", L"");//INFO: to avoid top curved corners on frame
		CREATESTRUCT* creation_info = (CREATESTRUCT*)lParam;
		mgr_data = (MANAGER*)creation_info->lpCreateParams;
		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE);

		SetupMgr(hWnd,hInstance,FRAME, *mgr_data, manager_font); //TODO(fran): resolve how to deal with font initialization

		//TODO(fran): SET SLIDERS AND ALSO OUTMGR VALUES (now they're inside SetupMgr, TAKE THEM OUT, setup should only know about creating controls)

		LANGUAGE_MANAGER::Instance().AddDynamicText(hWnd, SCV_MANAGER_LANG_DYNAMIC_UPDATE);

		//Create procedure to add things to the "non client" area
		RECT rc;
		GetWindowRect(hWnd, &rc);

		float button_height = (float)FRAME.caption_height;
		float button_width = button_height * 16.f / 9.f;

		//TODO(fran): send normal WM_CLOSE and associate messages from caption buttons, having extra msgs is unnecessary
		HWND close_button = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | WS_MAXIMIZEBOX
			, (int)(RECTWIDTH(rc) - button_width - FRAME.right_border), 0, (int)button_width, (int)button_height, hWnd, (HMENU)SCV_CUSTOMFRAME_CLOSE, hInstance, NULL);
		//SetWindowLongPtr(close_button, GWL_USERDATA, CROSS_ICON);
		SetWindowSubclass(close_button, ControlProcedures::Instance().CaptionButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
		TOOLTIP_REPO::Instance().CreateToolTip(close_button, SCV_LANG_CLOSE);


		HWND minimize_button = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | WS_MINIMIZEBOX
			, (int)(RECTWIDTH(rc) - button_width*2 - FRAME.right_border), 0, (int)button_width, (int)button_height, hWnd, (HMENU)SCV_CUSTOMFRAME_MINIMIZE, hInstance, NULL);
		//SetWindowLongPtr(minimize_button, GWL_USERDATA, MINIMIZE_ICON);
		SetWindowSubclass(minimize_button, ControlProcedures::Instance().CaptionButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
		TOOLTIP_REPO::Instance().CreateToolTip(minimize_button, SCV_LANG_MINIMIZE);

		//

		// Synchronization

		// Initialize mutexes
		//thread_data.output_wnd_ready_mutex = CreateMutex(NULL/*default security attributes*/,FALSE/*initially not owned*/,NULL/*unnamed*/);

		//if (thread_data.output_wnd_ready_mutex == NULL)
		//{
		//	ShowClosingAppError(SCV_LANG_ERROR_CREATEMUTEX);
		//	Assert(0); 
		//}

		//Load values for thread_data
		Assert(KNOWN_WINDOWS.veil);
		thread_data.output_wnd = KNOWN_WINDOWS.veil; //TODO(fran): check the veil exists, otherwise wait for it. We could also just say veil always starts before manager

		thread_data.next_frame_mutex = CreateMutex(NULL, FALSE, NULL);

		if (thread_data.next_frame_mutex == NULL)
		{
			ShowClosingAppError(SCV_LANG_ERROR_CREATEMUTEX);
			Assert(0);
		}

		if (!mgr_data->is_turned_on) {
			DWORD mut_ret = WaitForSingleObject(thread_data.next_frame_mutex, INFINITE);
			Assert(mut_ret == WAIT_OBJECT_0);
		}

		thread_data.worker_finished_mutex = CreateMutex(NULL, FALSE, NULL);

		if (thread_data.worker_finished_mutex == NULL)
		{
			ShowClosingAppError(SCV_LANG_ERROR_CREATEMUTEX);
			Assert(0);
		}

		//thread_data.events.UnexpectedErrorEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr); //INFO: NO error on implicit conversion from HANDLE to bool, bools are dangerous

		//Create worker thread that will generate the image to display
		DWORD ThreadID;
		worker_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkerThread, &thread_data, 0, &ThreadID);
		//

		break;
	}
	case WM_DISPLAYCHANGE:
	{
		// Resize the Veil window //TODO(fran): can I do it like this?
		ShowWindow(GetWindow(hWnd, GW_OWNER), SW_MAXIMIZE);
		// Tell output manager that window size has changed
		thread_data.output_mgr.WindowResize(); //Resize output mgr
		break;
	}
	//INFO IMPORTANT TODO: trackbar can use TBS_TRANSPARENTBKGND flag on creation and asks parent to paint its background by WM_PRINTCLIENT 
	//https://docs.microsoft.com/en-us/windows/win32/controls/trackbar-control-reference
	//https://docs.microsoft.com/en-us/windows/win32/controls/custom-draw-values
	//https://docs.microsoft.com/en-us/windows/win32/controls/wm-notify
	//https://docs.microsoft.com/en-us/windows/win32/controls/nm-customdraw-trackbar
	//case WM_NOTIFY:
	//{
	//	NMHDR* msg_info = (NMHDR*) lParam;
	//	switch (msg_info->code) {
	//	case NM_CUSTOMDRAW: //TODO: I can custom draw the trackbar from here
	//	{
	//		//TODO: check it is a trackbar otherwise defwindowproc
	//		NMCUSTOMDRAW* custom_draw = (NMCUSTOMDRAW*)lParam; 
	//		SetBkColor(custom_draw->hdc, RGB(255, 0, 0));
	//		switch (custom_draw->dwDrawStage)
	//		{
	//		case CDDS_PREPAINT: return CDRF_DODEFAULT;
	//		default:
	//			break;
	//		}
	//	}
	//	default: return DefWindowProc(hWnd, message, wParam, lParam);
	//	}
	//	break;
	//}
	case WM_PAINT: 
	{
		return PaintCaption(hWnd,FRAME);
	}
	case WM_NCACTIVATE: //stuff for custom frame
	{
		return TRUE;
	}
	case WM_NCHITTEST: //stuff for custom frame
	{
		return HitTestNCA(hWnd, wParam, lParam,FRAME);
	}
	case WM_NCCALCSIZE: //stuff for custom frame
	{
		if (wParam == TRUE) {
			NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

			pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
			pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
			pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
			pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;
			return 0;
		}
		else return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_NCPAINT: //stuff for custom frame
	{
		return 0;
	}
	case SCV_MANAGER_UPDATE_THRESHOLD_OPACITY:
	{
		SendMessage(hWnd, WM_HSCROLL, MAKELONG(TB_PAGEDOWN, 0), (LPARAM)get_child_by_id(hWnd,SCV_MANAGER_TOOLTIP_THRESHOLD_SLIDER));
		SendMessage(hWnd, WM_HSCROLL, MAKELONG(TB_PAGEDOWN, 0), (LPARAM)get_child_by_id(hWnd,SCV_MANAGER_TOOLTIP_OPACITY_SLIDER));
		break;
	}
	case SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF:
	{
		HWND TurnOnOff = get_child_by_id(hWnd, SCV_MANAGER_TURN_ON_OFF);
		//const WCHAR* turnedOnOffText;
		if (mgr_data->is_turned_on) {
			//turnedOnOffText = RST(SCV_LANG_MGR_TURN_OFF).c_str();
			SetWindowTextW(TurnOnOff, RCS(SCV_LANG_MGR_TURN_OFF));
			//TODO(fran): I think I should clear the hWnd also, leave it all transparent, or wait for a new updatelayeredwindow before sending show
		}
		else {
			//turnedOnOffText = RST(SCV_LANG_MGR_TURN_ON).c_str();
			SetWindowTextW(TurnOnOff, RCS(SCV_LANG_MGR_TURN_ON));
		}
		//SetWindowTextW(TurnOnOff, turnedOnOffText);
		InvalidateRect(TurnOnOff, NULL, TRUE);
		break;
	}
	case SCV_MANAGER_LANG_DYNAMIC_UPDATE: //Message sent for windows that have controls that need more than the supported functions from LANGUAGE_MANAGER
	{
		SendMessage(hWnd, SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF, 0, 0);
		SendMessage(hWnd, SCV_MANAGER_UPDATE_THRESHOLD_OPACITY, 0, 0);
		break;
	}
	case WM_HOTKEY:
	{
		UINT keyModifiers = LOWORD(lParam);
		UINT virtualKey = HIWORD(lParam);

		SETTINGS settings;
		//TODO(fran): can I use SendDlgItemMessage instead?????-----I dont think so
		SendMessage(KNOWN_WINDOWS.settings, SCV_SETTINGS_GET_SETTINGS, (WPARAM)&settings, NULL);

		//TODO(fran): right now that we only have one hotkey it is pointless to do this check I think
		if (((keyModifiers^MOD_NOREPEAT) == (settings.hotkey.mods^MOD_NOREPEAT)) && (virtualKey == settings.hotkey.vk)) {
			ShowWindow(hWnd, SW_SHOW);
		}
		break;
	}
	case WM_HSCROLL: //Manage Slider
	//INFO: horizontal sliders send WM_HSCROLL, vertical ones send WM_VSCROLL
	{
		switch (LOWORD(wParam)) {//TODO(fran): better way to handle multiple sliders, they must have a way to set an ID as I think saw in the example
			case TB_PAGEDOWN:
			case TB_PAGEUP:
			case TB_ENDTRACK:
			case TB_LINEDOWN:
			case TB_LINEUP:
			case TB_BOTTOM:
			case TB_THUMBPOSITION:
			case TB_THUMBTRACK:
			case TB_TOP:
				HWND slider = (HWND)lParam;
				int pos = SendMessage(slider, TBM_GETPOS, 0, 0); //TODO(fran): why do I cast to float??
				if (GetDlgCtrlID(slider) == SCV_MANAGER_TOOLTIP_THRESHOLD_SLIDER) {
					//TODO(fran): doing enumchildwindows all the time is probably very expensive, we might need to keep a list of important hwnds
					//static int last_thresh_pos = -1; //TODO(fran): I'm not sure this is even better, the problem must be somewhere else
					//if(pos!= last_thresh_pos)
						SetWindowTextW(get_child_by_id(hWnd,SCV_MANAGER_THRESHOLD_TITLE), fmt::format(RS(SCV_LANG_MGR_THRESHOLD), pos).c_str()); //TODO(fran): implement my own static text control
					thread_data.output_mgr.SetThreshold(pos/100.f);
					//last_thresh_pos = pos;
				}
				else if (GetDlgCtrlID(slider) == SCV_MANAGER_TOOLTIP_OPACITY_SLIDER) {
					//static int last_opac_pos = -1;
					//if (pos != last_opac_pos)
						SetWindowTextW(get_child_by_id(hWnd, SCV_MANAGER_OPACITY_TITLE), fmt::format(RS(SCV_LANG_MGR_OPACITY), pos).c_str());
					thread_data.output_mgr.SetOpacity((100-pos) / 100.f);
					//last_opac_pos = pos;
				}
				break;
		}
		break;
	}
	case WM_COMMAND: 
	{
		//switch (HIWORD(wParam)) {
		//case EN_CHANGE:
		//{
		//	HWND hotkeyhandle = (HWND)lParam;
		//	WORD hotkeyval = SendMessageW(hotkeyhandle, HKM_GETHOTKEY, 0, 0);
		//	break;
		//}
		//}
		switch (LOWORD(wParam)) 
		{
		case SCV_CUSTOMFRAME_CLOSE:  //TODO(fran): can I make this guys send the usual WM_CLOSE msg and others?-------------------------------------------
		{
			DestroyWindow(hWnd);
			break;
		}
		case SCV_CUSTOMFRAME_MINIMIZE:
		{
			//CloseWindow(hWnd);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		case SCV_MANAGER_SECRET_ABOUT:
		{
			std::wstring copyright = GetVersionInfo(NULL, SCV_VERSION_INFO, L"LegalCopyright");
			std::wstring copyright_year = first_number_in_string(copyright);
			std::wstring about_msg = L"Franco Badenas Abal ©";
			if (copyright_year != L"") about_msg += L" " + copyright_year;
			about_msg += L"\n\n";
			about_msg += RS(SCV_LANG_VERSION);
			about_msg += L" " + GetVersionInfo(NULL, SCV_VERSION_INFO, L"ProductVersion");//TODO(fran): if no version found put message saying that or dont show version
			//TODO(fran): make it so only one about window can exist at any single time, without having to put hwnd parameter which locks the manager
			MessageBoxW(NULL, about_msg.c_str(), RCS(SCV_LANG_ABOUT), MB_TOPMOST);
			break;
		}
		case SCV_MANAGER_SETTINGS:
		{

			ShowWindow(KNOWN_WINDOWS.settings, SW_SHOW);
			break;
		}
		case SCV_MANAGER_TURN_ON_OFF: //Show/Hide the Veil
		{
			mgr_data->is_turned_on = !mgr_data->is_turned_on;
			if (mgr_data->is_turned_on) {
				//TODO(fran): could it ever happen that we didnt already own the mutex?
				ReleaseMutex(thread_data.next_frame_mutex);
				ShowWindow(GetWindow(hWnd,GW_OWNER), SW_SHOW);
			}
			else {
				ShowWindow(GetWindow(hWnd, GW_OWNER), SW_HIDE);
				thread_data.output_mgr.RestartTextures();

				//TODO(fran): could it ever happen that we already had the mutex owned?
				DWORD mut_ret = WaitForSingleObject(thread_data.next_frame_mutex, INFINITE);
				Assert(mut_ret == WAIT_OBJECT_0);

			}
			SendMessage(hWnd, SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF, 0, 0); 
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case SCV_MANAGER_MODIFY_TRAY:
	{
		if ((BOOL)wParam) {
			if (!TRAY_HANDLER::Instance().CreateTrayIcon(hWnd, 1, LOGO_ICON, SCV_MANAGER_TRAY, SCV_LANG_TRAY_TIP))
				MessageBox(hWnd, RCS(SCV_LANG_ERROR_TRAY_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
		}
		else {
			TRAY_HANDLER::Instance().DestroyTrayIcon(hWnd, 1);
		}
		break;
	}
	case SCV_MANAGER_TRAY: //Tray icon handling
	{
		switch (lParam) {
		case WM_LBUTTONDOWN: 
		{
			SendMessage(hWnd, WM_COMMAND, MAKELONG(SCV_MANAGER_TURN_ON_OFF,0), 0);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			//TODO(fran): should we do the animation on startup?
			//TODO(fran): add support for taskbar icon
			//TODO(fran): we need to check if the window is already shown and do nothing in that case
			//TODO(fran): do further tests to be sure IsWindowVisible will always work here
			SETTINGS settings;
			SendMessage(KNOWN_WINDOWS.settings, SCV_SETTINGS_GET_SETTINGS, (WPARAM)&settings, NULL);
			if (settings.show_tray_icon && !IsWindowVisible(hWnd)) {
				//Animate closing to tray
				//Pd AnimateWindow is awful
				//Thanks to: https://www.codeproject.com/Articles/735/Minimizing-windows-to-the-System-Tray
				RestoreWndFromTray(hWnd);
			}
			else
				ShowWindow(hWnd, SW_SHOW);

			break;
		}
		}
		break;
	}
	//case WM_NOTIFY: {
	//	LPNMHDR header = reinterpret_cast<LPNMHDR>(lParam);
	//	switch (header->code) {
	//	case BCN_HOTITEMCHANGE: {
	//		NMBCHOTITEM* hot_item = reinterpret_cast<NMBCHOTITEM*>(lParam);

	//		// Handle to the button
	//		HWND button_handle = header->hwndFrom;

	//		// ID of the button, if you're using resources
	//		UINT_PTR button_id = header->idFrom;

	//		// You can check if the mouse is entering or leaving the hover area
	//		bool entering = hot_item->dwFlags & HICF_ENTERING;
	//		break;
	//	}
	//	}
	//	break;
	//}
	case WM_CTLCOLORSTATIC:
	{
		HDC staticDC = (HDC)wParam;
		SetTextColor(staticDC, ControlProcedures::Instance().Get_HighlightColor());
		SetBkColor(staticDC, ControlProcedures::Instance().Get_BackgroundColor());
		return (INT_PTR)ControlProcedures::Instance().Get_BackgroundBrush();
	}
	//case WM_CTLCOLORBTN:{}
	//case WM_DRAWITEM: { //INFO: I'm using it only to draw buttons

	//	return DrawButton((DRAWITEMSTRUCT*)lParam,wParam);

	//}
	case WM_CLOSE:
	{

		SETTINGS settings;
		SendMessage(KNOWN_WINDOWS.settings, SCV_SETTINGS_GET_SETTINGS, (WPARAM)&settings, NULL);

		SendMessage(KNOWN_WINDOWS.settings, WM_CLOSE, 0, 0);

		if (settings.show_tray_icon) { //Check if the tray icon is enabled
			//Animate closing to tray
			//Pd AnimateWindow is awful
			//Thanks to: https://www.codeproject.com/Articles/735/Minimizing-windows-to-the-System-Tray
			MinimizeWndToTray(hWnd);
		}
		else
			ShowWindow(hWnd, SW_HIDE);
		break;
	}
#if 0
	case WM_SIZE: //will we accept resizing?
	{
		break;
	}
#endif
	case WM_DESTROY: 
	{
		static BOOL already_destroyed=FALSE;
		if (!already_destroyed) {
			already_destroyed = TRUE;
			//Save MANAGER* //TODO(fran): this should be done when things change, aka during the duration of the window, NOT here at the end
			RECT rec;
			GetWindowRect(hWnd, &rec);
			POINT mgrpos = { rec.left,rec.top };
			SIZE virtualscreensize = { GetSystemMetrics(SM_CXVIRTUALSCREEN),GetSystemMetrics(SM_CYVIRTUALSCREEN) };//TODO(fran): check this is what we want to be calling on multi-monitor
			int thresholdpos = (int)SendDlgItemMessage(hWnd, SCV_MANAGER_TOOLTIP_THRESHOLD_SLIDER, TBM_GETPOS, NULL, NULL);
			int opacitypos = (int)SendDlgItemMessage(hWnd, SCV_MANAGER_TOOLTIP_OPACITY_SLIDER, TBM_GETPOS, NULL, NULL);

			mgr_data->previous_manager_position = mgrpos;
			mgr_data->previous_screen_size = virtualscreensize;
			mgr_data->slider_opacity_pos = opacitypos;
			mgr_data->slider_threshold_pos = thresholdpos;

			//TODO(fran): Establish this as the only application exit path
			//INFO(fran): here destroy everything and close the app
			//Wait for image processing thread to exit
			thread_data.terminate = TRUE;

			if (!mgr_data->is_turned_on) ReleaseMutex(thread_data.next_frame_mutex);
			WaitForSingleObject(thread_data.worker_finished_mutex, INFINITE);
			ReleaseMutex(thread_data.worker_finished_mutex);
			CloseHandle(thread_data.worker_finished_mutex);

			// Make sure all other threads have exited
			//thread_data.events.TerminateThreadsEvent = true; //TODO(fran): pointless?
			thread_data.thread_mgr.WaitForThreadTermination();//TODO(fran): this should not be here

			// Clean up
			//CloseHandle(thread_data.events.UnexpectedErrorEvent);
			//CloseHandle(thread_data.events.ExpectedErrorEvent);
			//CloseHandle(thread_data.events.TerminateThreadsEvent);

			CloseHandle(worker_thread_handle);
			CloseHandle(thread_data.next_frame_mutex);

			//WaitForSingleObject(thread_data.output_wnd_ready_mutex, INFINITE);
			//ReleaseMutex(thread_data.output_wnd_ready_mutex);
			//CloseHandle(thread_data.output_wnd_ready_mutex);

			//


			if (GetWindow(hWnd, GW_OWNER) != NULL) { DestroyWindow(GetWindow(hWnd, GW_OWNER)); }
			//INFO IMPORTANT(fran): DestroyWindow exits this proc goes and destroys the window and then comes back through a new WM_DESTROY,
			//is this because the window we're destroying is our parent, and it's trying to destroy us? Anyway WM_DESTROY GETS EXECUTED TWICE
			//that's why CloseHandle throws invalid handle
			//TODO(fran): can we solve this problem without the static BOOL?
		}
		
		TRAY_HANDLER::Instance().DestroyTrayIcon(hWnd, 1);
		DeleteObject(manager_font);
		
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


//LRESULT CALLBACK StaticTextProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
//	switch (Msg) {
//	case WM_PAINT:
//	{
//		PAINTSTRUCT ps;
//		HDC hdc = BeginPaint(hWnd, &ps);
//
//		WCHAR text[200];
//		SendMessage(hWnd, WM_GETTEXT, 200, (LPARAM)text);
//
//		RECT rc;
//		GetClientRect(hWnd, &rc);
//
//		SetBkColor(hdc, RGB(0, 0, 0));
//		SetTextColor(hdc, RGB(255, 255, 255));
//		HFONT test = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
//		SelectObject(hdc, (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0));
//
//		DrawText(hdc, text, -1, &rc, DT_EDITCONTROL | DT_LEFT | DT_VCENTER | DT_SINGLELINE);
//
//		EndPaint(hWnd, &ps);
//		return 0;
//	}
//	default: return DefWindowProc(hWnd, Msg, wParam, lParam);
//	}
//	return 0;
//}
