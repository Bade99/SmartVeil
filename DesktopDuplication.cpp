// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#include "resource.h"

#include <limits.h>

#include "DisplayManager.h"
#include "DuplicationManager.h"
#include "OutputManager.h"
#include "ThreadManager.h"
#include <string>
#include <commctrl.h>
#include <strsafe.h> //for stringcchcopy
#include <vector>
#include <Shlobj.h> //tooltips
#include <cwctype> // iswspace
#include <algorithm> // remove_if
#include <map>
#include <sstream>
#include "utils.cpp"
#include <uxtheme.h>// setwindowtheme
#include "TOOLTIP_REPO.h"
#include "TRAY_HANDLER.h"
#include "LANGUAGE_MANAGER.h"
#include "dwmapi.h"
#include "Windowsx.h"
#include "vssym32.h"
#include "startup_and_settings.cpp"
#include "ControlProcedures.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"UxTheme.lib")// setwindowtheme
#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"Version.lib") //for VerQueryValue

#define SCV_TRAY (WM_USER+2)
#define SCV_TURN_ON_OFF (WM_USER+4)
#define SCV_SAVE_HOTKEY (WM_USER+5)
#define SCV_THRESHOLD_TITLE (WM_USER+8)
#define SCV_TOOLTIP_THRESHOLD_SLIDER (WM_USER+9)
#define SCV_SECRET_ABOUT (WM_USER+10)
#define SCV_SETTINGS (WM_USER+11)
#define SCV_TOOLTIP_OPACITY_SLIDER (WM_USER+12)
#define SCV_SETTINGS_LANG_COMBO (WM_USER+13)
#define SCV_SETTINGS_LANG_TITLE (WM_USER+14)
#define SCV_SETTINGS_DANGEROUS_SLIDER (WM_USER+15)
#define SCV_SETTINGS_MANAGER_POS (WM_USER+16)
#define SCV_SETTINGS_MANAGER_ON_STARTUP (WM_USER+17)
#define SCV_SETTINGS_SHOW_TOOLTIPS (WM_USER+18)
#define SCV_SETTINGS_SHOW_TRAY (WM_USER+19)
#define SCV_SETTINGS_VEIL_STARTUP_TITLE (WM_USER+20)
#define SCV_SETTINGS_VEIL_STARTUP_COMBO (WM_USER+21)
#define SCV_SETTINGS_START_WITH_WINDOWS (WM_USER+22)
#define SCV_SETTINGS_SAVE (WM_USER+23)
#define SCV_SETTINGS_HOTKEY (WM_USER+24)
#define SCV_UPDATE_COUNTER (WM_USER+26)
#define SCV_UPDATE_COUNTER_TEXT (WM_USER+27)
#define SCV_MANAGER_CREATE_TRAY (WM_USER+28)//TODO(fran): no need to have two defines, could send true or false through param
#define SCV_MANAGER_REMOVE_TRAY (WM_USER+29)
#define SCV_VEIL_SHOW_MGR (WM_USER+30)
#define SCV_LANG_DYNAMIC_UPDATE (WM_USER+31)
#define SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF (WM_USER+32)
#define SCV_CUSTOMFRAME_CLOSE (WM_USER+33)
#define SCV_CUSTOMFRAME_MINIMIZE (WM_USER+34)

struct CUSTOM_FRAME { //INFO: defines the sizes for the custom window frame
	int caption_height;
	int left_border;
	int right_border;
	int bottom_border;
	HBRUSH caption_bk_brush;
} FRAME; //INFO: every frame will be like this one

//INFO: Can and probably will return left & top values different from 0 since now that we draw a custom frame the whole window rect is ours
BOOL GetMyClientRect(HWND hwnd, RECT* rc) {
	BOOL res = GetWindowRect(hwnd, rc);//INFO: could use getclientrect, though values will be wrong until the hwnd destroys its non client area
	if (res) {
		rc->right = RECTWIDTH((*rc)) - FRAME.right_border;
		rc->bottom = RECTHEIGHT((*rc)) - FRAME.bottom_border;
		rc->left = FRAME.left_border;
		rc->top = FRAME.caption_height;
	}
	return res;
}

#define MS_UPDATE 0 //show veil update rate in milliseconds

//Limit veil "fps"
#define LIMIT_UPDATE 0
#if LIMIT_UPDATE 
double TargetFPS = 100;
double TargetMs = 1000.0 / TargetFPS;//TODO(fran): do we leave this as a global?
#endif

//TODOs:
//·Alt+F4 doesnt close the app cause of the closing system we currently have, change it now that we have a close and a minimize button
//·Checkbox for Minimize button: when tray icon is disabled show app icon on taskbar?
//?·Bren said that left click is related with opening stuff so it'd make more sense to use the left click for opening the manager and right for turn on/off
//·I dont think ProcessCmdline is doing any use to me
//·Icons on buttons are not correctly centered
//·Create one big square for the tooltips of threshold and opacity each one encompassing the area from the beginning of the text to the end of the slider?
//·Adjust to realtime resolution change
//·Fix CreateToolTipForRect, doesnt work now that each tooltip has to use the messageID to retrieve the string, maybe some flag is missing?
//·When using the wheel over a slider the values go in reverse
//·When sliders are pressed there is a white border around it, why?, also the thingy that moves the slider is now white when moved, why? looks better though
//·Fix incorrect string retrieval for hotkey control, I think we need to add an extended flag to some keys
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
//·Hotkey control goes insane when inputting with japanese keyboard
//·Way to know when a window is visible, so for example we dont send the frame update info to the settings
//·Find out why just the mouse movement is causing the highest of cpu and gpu usage, look into the other cpp files
//·IMPORTANT: the fact that we update our veil means there is a new frame that windows has to present, therefore we are always generating more
// and more frames non stop, can we fix this somehow? ie skip one frame update each time
//LOOKs:
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

// Below are lists of errors expect from Dxgi API calls when a transition event like mode change, PnpStop, PnpStart
// desktop switch, TDR or session disconnect/reconnect. In all these cases we want the application to clean up the threads that process
// the desktop updates and attempt to recreate them.
// If we get an error that is not on the appropriate list then we exit the application

// These are the errors we expect from general Dxgi API due to a transition
HRESULT SystemTransitionsExpectedErrors[] = {
                                                DXGI_ERROR_DEVICE_REMOVED,
                                                DXGI_ERROR_ACCESS_LOST,
                                                static_cast<HRESULT>(WAIT_ABANDONED),
                                                S_OK                                    // Terminate list with zero valued HRESULT
                                            };

// These are the errors we expect from IDXGIOutput1::DuplicateOutput due to a transition
HRESULT CreateDuplicationExpectedErrors[] = {
                                                DXGI_ERROR_DEVICE_REMOVED,
                                                static_cast<HRESULT>(E_ACCESSDENIED),
                                                DXGI_ERROR_UNSUPPORTED,
                                                DXGI_ERROR_SESSION_DISCONNECTED,
                                                S_OK                                    // Terminate list with zero valued HRESULT
                                            };

// These are the errors we expect from IDXGIOutputDuplication methods due to a transition
HRESULT FrameInfoExpectedErrors[] = {
                                        DXGI_ERROR_DEVICE_REMOVED,
                                        DXGI_ERROR_ACCESS_LOST,
                                        S_OK                                    // Terminate list with zero valued HRESULT
                                    };

// These are the errors we expect from IDXGIAdapter::EnumOutputs methods due to outputs becoming stale during a transition
HRESULT EnumOutputsExpectedErrors[] = {
                                          DXGI_ERROR_NOT_FOUND,
                                          S_OK                                    // Terminate list with zero valued HRESULT
                                      };


//
// Forward Declarations
//
DWORD WINAPI WorkerThread(LPVOID Param);
DWORD WINAPI DDProc(_In_ void* Param);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool ProcessCmdline(_Out_ INT* Output);
std::wstring GetExePath();
LSTATUS RegisterProgramOnStartup(std::wstring exe_path);
LSTATUS UnregisterProgramFromStartup();
//void ShowHelp();
//BOOL DrawButton(DRAWITEMSTRUCT* pDIS, WPARAM controlID);
LRESULT PaintCaption(HWND);
LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndMgrProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndSettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//
// Class for progressive waits
//
typedef struct
{
    UINT    WaitTime;
    UINT    WaitCount;
}WAIT_BAND;

#define WAIT_BAND_COUNT 3
#define WAIT_BAND_STOP 0

class DYNAMIC_WAIT
{
    public :
		DYNAMIC_WAIT();
        ~DYNAMIC_WAIT();

        void Wait();

    private :

    static const WAIT_BAND   m_WaitBands[WAIT_BAND_COUNT];

    // Period in seconds that a new wait call is considered part of the same wait sequence
    static const UINT       m_WaitSequenceTimeInSeconds = 2;

    UINT                    m_CurrentWaitBandIdx;
    UINT                    m_WaitCountInCurrentBand;
    LARGE_INTEGER           m_QPCFrequency;
    LARGE_INTEGER           m_LastWakeUpTime;
    BOOL                    m_QPCValid;
};
const WAIT_BAND DYNAMIC_WAIT::m_WaitBands[WAIT_BAND_COUNT] = {
                                                                 {250, 20},
                                                                 {2000, 60},
                                                                 {5000, WAIT_BAND_STOP}   // Never move past this band
                                                             };

DYNAMIC_WAIT::DYNAMIC_WAIT() : m_CurrentWaitBandIdx(0), m_WaitCountInCurrentBand(0)
{
    m_QPCValid = QueryPerformanceFrequency(&m_QPCFrequency);
    m_LastWakeUpTime.QuadPart = 0L;
}

DYNAMIC_WAIT::~DYNAMIC_WAIT()
{
}

void DYNAMIC_WAIT::Wait()
{
	LARGE_INTEGER CurrentQPC = { 0 };

    // Is this wait being called with the period that we consider it to be part of the same wait sequence
    QueryPerformanceCounter(&CurrentQPC);
    if (m_QPCValid && (CurrentQPC.QuadPart <= (m_LastWakeUpTime.QuadPart + (m_QPCFrequency.QuadPart * m_WaitSequenceTimeInSeconds))))
    {
        // We are still in the same wait sequence, lets check if we should move to the next band
        if ((m_WaitBands[m_CurrentWaitBandIdx].WaitCount != WAIT_BAND_STOP) && (m_WaitCountInCurrentBand > m_WaitBands[m_CurrentWaitBandIdx].WaitCount))
        {
            m_CurrentWaitBandIdx++;
            m_WaitCountInCurrentBand = 0;
        }
    }
    else
    {
        // Either we could not get the current time or we are starting a new wait sequence
        m_WaitCountInCurrentBand = 0;
        m_CurrentWaitBandIdx = 0;
    }

    // Sleep for the required period of time
    Sleep(m_WaitBands[m_CurrentWaitBandIdx].WaitTime);

    // Record the time we woke up so we can detect wait sequences
    QueryPerformanceCounter(&m_LastWakeUpTime);
    m_WaitCountInCurrentBand++;
}

//
// Globals
//
OUTPUTMANAGER OutMgr;

INT SingleOutput;

THREADMANAGER ThreadMgr;
RECT DeskBounds;
UINT OutputCount;

// Synchronization
HANDLE UnexpectedErrorEvent = nullptr;
HANDLE ExpectedErrorEvent = nullptr;
HANDLE TerminateThreadsEvent = nullptr;

bool isTurnedOn = true;

SETTINGS CurrentValidSettings;

//Hotkey management
//Manual hotkey work
UINT CurrentValidHotkeyModifiers;
UINT CurrentValidHotkeyVirtualKey;

// Windows
HWND WindowHandle = nullptr; // Veil window
HWND WindowManagerHandle = nullptr; // Manager window
HWND WindowSettingsHandle = nullptr; // Settings window


HANDLE start_mutex;//signals the worker thread that the main window is ready to receive data to display
HANDLE process_next_frame_mutex;//indicates that the veil is turned on, therefore the worker thread can send data to display
HANDLE thread_finished_mutex;
BOOL ProgramFinished = FALSE;

//Path to the file where we store our startup data
STARTUP_INFO_PATH info_path;

std::wstring GetExePath() {
	WCHAR exe_path[MAX_PATH];
	GetModuleFileNameW(NULL, exe_path, MAX_PATH);//INFO: last param is size of buffer in TCHAR
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		//path is too long for the buffer, it must be using the \\?\ prefix
		WCHAR long_exe_path[32767];//INFO: documentation says this max value is approximate
		GetModuleFileNameW(NULL, long_exe_path, 32767);//TODO(fran): check whether this works or needs something extra
		return long_exe_path;
	}
	return exe_path;
}

//·Returns ERROR_SUCCESS if succeeded
//·On failure returns error code, use FormatMessage with FORMAT_MESSAGE_FROM_SYSTEM to get error description
LSTATUS RegisterProgramOnStartup(std::wstring exe_path) {
	
	WCHAR exe_stored_path[32767];
	DWORD string_size = 32767 * sizeof(wchar_t);
	//INFO: when reggetvalue returns this dword will be the length of the copied string, if the buffer is not large enough this dword will be the
	// required size
	LSTATUS res = RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"Smart Veil",
		RRF_RT_REG_SZ, NULL, exe_stored_path, &string_size);//add RRF_ZEROONFAILURE?
	BOOL UpdateReg = FALSE;

	if (res == ERROR_SUCCESS) {
		if (wcscmp(exe_path.c_str(), exe_stored_path) != 0) UpdateReg = TRUE;
	}
	else if (res == ERROR_MORE_DATA) {
		//INFO IMPORTANT: we know that our max array size in GetExePath is 32767 so if something bigger is in the registry
		// that means it's never gonna be the same
		UpdateReg = TRUE;
	}
	else {
		//function failed and res_get = some error code
		UpdateReg = TRUE;
	}
	if (UpdateReg) {
		HKEY OnStartup=NULL;
		//DWORD disposition;//INFO: can be REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY

		//first you create the registry if it doesnt already exist
		res = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", NULL, NULL,
			REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE| KEY_SET_VALUE| STANDARD_RIGHTS_REQUIRED, NULL, &OnStartup, NULL);
		if (res != ERROR_SUCCESS) {
			return res;
		}
		//now you modify the data of a value inside that registry
		res = RegSetValueExW(OnStartup, L"Smart Veil", NULL, REG_SZ, (BYTE*)exe_path.c_str(), (exe_path.length() + 1) * sizeof(wchar_t));
		RegCloseKey(OnStartup);
	}
	//TODO(fran): add error msg or something in case we receive another value?, documentation says those are the only values returned
	return res;
}

//If successful returns ERROR_SUCCESS, otherwise returns the error value, use FormatMessage with FORMAT_MESSAGE_FROM_SYSTEM flag to get error description 
LSTATUS UnregisterProgramFromStartup() {
	LSTATUS res = RegDeleteKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"Smart Veil");
	return res;
}

inline void ShowClosingAppError(UINT error_stringID) {
	std::wstring error_msg = RS(error_stringID);
	error_msg += L" " + std::to_wstring(GetLastError()) + L"\n" + RS(SCV_LANG_ERROR_CLOSING_APP);
	MessageBoxW(NULL, error_msg.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
}

int GetBasicWindowCaptionHeight(HINSTANCE hInstance,POINT aprox_pos) {//TODO(fran): make this work for multi-monitor
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

	int border = ((float)(RECTWIDTH(rw) - RECTWIDTH(rc))) / 2.f;

	int caption_height = RECTHEIGHT(rw) - RECTHEIGHT(rc) - border;

	DestroyWindow(hWnd);
	UnregisterClass(L"Franco Test Class Height", hInstance);

	return caption_height;
}

//·You can search for:
//"CompanyName"
//"FileDescription"
//"FileVersion"
//"InternalName"
//"LegalCopyright"
//"OriginalFilename"
//"ProductName"
//"ProductVersion"
std::wstring GetVersionInfo(HMODULE hLib, UINT versionID, WCHAR* csEntry)
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

				swprintf(fileEntry,256, L"\\VarFileInfo\\Translation");
				retVal = VerQueryValue(versionInfo, fileEntry, &retbuf, (UINT *)&vLen);
				if (retVal && vLen == 4)
				{
					memcpy(&langD, retbuf, 4);
					swprintf(fileEntry,256, L"\\StringFileInfo\\%02X%02X%02X%02X\\%s",
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
			}
		}

		UnlockResource(hGlobal);
		FreeResource(hGlobal);
	}

	return csRet;

	//TODO(fran): instead of using this code use this one which allows for multiple languages, we will need to check the ones it has
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

//
// Program entry point
//
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	std::wstring VeilClassName = L"Smart Veil Franco Badenas Abal";
	std::wstring VeilName = L"Smart Veil, the veil itself";

	//Check that no instance is already running on this user session
	//TODO(fran): from my tests this system is user session independent, check that's true for every case, 
	// eg. admin - admin, admin - normal user, normal user - normal user
	HANDLE single_instance_mutex = CreateMutex(NULL, TRUE, L"Smart Veil Single Instance Mutex Franco Badenas Abal");
	if (single_instance_mutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
		//If an instance already exists try to show its manager to the user
		//INFO: other ways of solving the hwnd finding: http://www.flounder.com/nomultiples.htm
		HWND existingApp = FindWindow(VeilClassName.c_str(), VeilName.c_str());
		if (existingApp) PostMessage(existingApp, SCV_VEIL_SHOW_MGR, 0,0);
		return 0; // Exit the app
	}

	//Load startup info
	PWSTR folder_path;
	HRESULT folder_res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL,&folder_path);//TODO(fran): this is only supported on windows vista and above, will we ever care?
	//INFO: no retorna la ultima barra, la tenés que agregar vos, tambien te tenés que encargar de liberar el string
	Assert(SUCCEEDED(folder_res));
	info_path.known_folder = folder_path;
	CoTaskMemFree(folder_path);
	info_path.info_folder = L"Smart Veil";
	info_path.info_file = L"startup_info";
	info_path.info_extension = L"txt";//just so the user doesnt even have to open the file manually on windows

	STARTUP_INFO startup_info;

	std::wstring info_file_path = info_path.known_folder + L"\\" + info_path.info_folder + L"\\" + info_path.info_file + L"." + info_path.info_extension;
	HANDLE info_file_handle = CreateFileW(info_file_path.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE
		, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (info_file_handle != INVALID_HANDLE_VALUE) {
#define INFO_FILE_SIZE 5000 //TODO(fran): cleaner solution?
		WCHAR buf[INFO_FILE_SIZE];
		DWORD bytes_read;
		BOOL read_res = ReadFile(info_file_handle, buf, INFO_FILE_SIZE, &bytes_read, NULL);
#undef INFO_FILE_SIZE 

		if (read_res) {

			STARTUP_INFO_TEXT startup_info_text;
			std::map<std::wstring,std::wstring> info_mapped = mappify(buf, startup_info_text.separator);
			FillStartupInfo(info_mapped, startup_info);
		}
		else {
			DefaultStartupInfo(startup_info);
		}

		CloseHandle(info_file_handle);
	}
	else {
		DefaultStartupInfo(startup_info);
	}

	// Set values with the new startup_info data
	if (startup_info.show_veil_on_startup == VEIL_ON_STARTUP::YES) {
		isTurnedOn = TRUE;
	}
	else if (startup_info.show_veil_on_startup == VEIL_ON_STARTUP::NO){
		isTurnedOn = FALSE;
	}
	else if (startup_info.show_veil_on_startup == VEIL_ON_STARTUP::REMEMBER_LAST_STATE) {
		isTurnedOn = startup_info.is_turned_on;
	}
	else { isTurnedOn = startup_info.is_turned_on; }
	//INFO: nobody should check show_veil_on_startup or is_turned_on, they can directly continue using isTurnedOn
	
	CurrentValidHotkeyModifiers = startup_info.hotkey_modifiers;
	CurrentValidHotkeyVirtualKey = startup_info.hotkey_virtual_key;

	CurrentValidSettings.show_manager_on_startup = startup_info.show_manager_on_startup;
	CurrentValidSettings.show_tray_icon = startup_info.show_tray_icon;
	CurrentValidSettings.reduce_dangerous_slider_values = startup_info.reduce_dangerous_slider_values;
	CurrentValidSettings.remember_manager_position = startup_info.remember_manager_position;
	CurrentValidSettings.show_tooltips = startup_info.show_tooltips;
	CurrentValidSettings.start_with_windows = startup_info.start_with_windows;
	CurrentValidSettings.show_veil_on_startup = startup_info.show_veil_on_startup;
	CurrentValidSettings.language = startup_info.language;

	LANGUAGE_MANAGER::Instance().SetHInstance(hInstance);
	LANGUAGE_MANAGER::Instance().ChangeLanguage(CurrentValidSettings.language);
	TOOLTIP_REPO::Instance().SetHInstance(hInstance);
	TOOLTIP_REPO::Instance().ActivateTooltips(CurrentValidSettings.show_tooltips);

	if (CurrentValidSettings.start_with_windows) {
		//Always check that the value stored in the registry is the same as the value of this exe
		RegisterProgramOnStartup(GetExePath());
	}

// Initialize mutexes
	start_mutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed

	if (start_mutex == NULL)
	{
		ShowClosingAppError(SCV_LANG_ERROR_CREATEMUTEX);
		return 0;
	}

	process_next_frame_mutex = CreateMutex(NULL, FALSE, NULL);

	if (process_next_frame_mutex == NULL)
	{
		ShowClosingAppError(SCV_LANG_ERROR_CREATEMUTEX);
		return 0;
	}

	if (!isTurnedOn) {
		DWORD mut_ret = WaitForSingleObject(process_next_frame_mutex, INFINITE);
		Assert(mut_ret == WAIT_OBJECT_0);
	}

	thread_finished_mutex = CreateMutex(NULL, FALSE, NULL);

	if (thread_finished_mutex == NULL)
	{
		ShowClosingAppError(SCV_LANG_ERROR_CREATEMUTEX);
		return 0;
	}

	// Request ownership of mutex.
	DWORD mut_ret = WaitForSingleObject(start_mutex, INFINITE);
	if (mut_ret != WAIT_OBJECT_0) {
		ShowClosingAppError(SCV_LANG_ERROR_ACQUIREMUTEX);
		return 0;
	}

	bool CmdResult = ProcessCmdline(&SingleOutput);
	if (!CmdResult)
	{
		//ShowHelp();
		Assert(0);
		return 0;
	}

	// Event used by the threads to signal an unexpected error and we want to quit the app
	UnexpectedErrorEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!UnexpectedErrorEvent)
	{
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_UNEXPECTEDERROREVENT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
		return 0;
	}

	// Event for when a thread encounters an expected error
	ExpectedErrorEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!ExpectedErrorEvent)
	{
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_EXPECTEDERROREVENT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
		return 0;
	}

	// Event to tell spawned threads to quit
	TerminateThreadsEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!TerminateThreadsEvent)
	{
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_TERMINATETHREADSEVENT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
		return 0;
	}

	// Ensure that the common control DLL for hotkey is loaded. 
	INITCOMMONCONTROLSEX icex; // declare an INITCOMMONCONTROLSEX Structure
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_HOTKEY_CLASS; // set dwICC member to ICC_HOTKEY_CLASS    
	InitCommonControlsEx(&icex); // this loads the Hot Key control class.

	// Load simple cursor
    HCURSOR Cursor = nullptr;
    Cursor = LoadCursor(nullptr, IDC_ARROW);
    if (!Cursor)
    {
        ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_CURSOR_LOAD), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
        return 0;
    }

	HICON logo_icon;
	HICON logo_icon_small;
	LoadIconMetric(hInstance, MAKEINTRESOURCE(LOGO_ICON), LIM_LARGE, &logo_icon);
	LoadIconMetric(hInstance, MAKEINTRESOURCE(LOGO_ICON), LIM_SMALL, &logo_icon_small);

    // Register class
    WNDCLASSEXW Wc;
    Wc.cbSize           = sizeof(WNDCLASSEXW);
    Wc.style            = CS_HREDRAW | CS_VREDRAW;
    Wc.lpfnWndProc      = WndProc;
    Wc.cbClsExtra       = 0;
    Wc.cbWndExtra       = 0;
    Wc.hInstance        = hInstance;
    Wc.hIcon            = logo_icon;
    Wc.hCursor          = Cursor;
    Wc.hbrBackground    = nullptr;
    Wc.lpszMenuName     = nullptr;
    Wc.lpszClassName    = VeilClassName.c_str();
	Wc.hIconSm          = logo_icon_small;
    if (!RegisterClassExW(&Wc))
    {
        ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_WINDOWCLASS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
        return 0;
    }

    // Create window
	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.top = 0;
	WindowRect.bottom = 600;
	WindowRect.right = 800;
    WindowHandle = CreateWindowExW( WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
						   VeilClassName.c_str(), VeilName.c_str(),
                           WS_POPUP,
                           0, 0,
                           WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top,
                           nullptr, nullptr, hInstance, nullptr);
    if (!WindowHandle)
    {
        ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_FAIL);
        return 0;
    }

	//Controls' color setup
	ControlProcedures::Instance().Set_BackgroundColor(RGB(0, 0, 0));
	ControlProcedures::Instance().Set_HighlightColor(RGB(255, 255, 255));
	ControlProcedures::Instance().Set_PushColor(RGB(0, 110, 200));
	ControlProcedures::Instance().Set_MouseoverColor(RGB(0, 120, 215));

	std::wstring appManagerClassName = L"Smart Veil Manager Franco Badenas Abal";

	//Register Manager class
	WNDCLASSEXW WMc;
	WMc.cbSize = sizeof(WNDCLASSEXW);
	WMc.style = CS_HREDRAW | CS_VREDRAW;
	WMc.lpfnWndProc = WndMgrProc;
	WMc.cbClsExtra = 0;
	WMc.cbWndExtra = 0;
	WMc.hInstance = hInstance;
	WMc.hIcon = logo_icon;
	WMc.hCursor = Cursor;
	WMc.hbrBackground = ControlProcedures::Instance().Get_BackgroundBrush();
	WMc.lpszMenuName = nullptr;
	WMc.lpszClassName = appManagerClassName.c_str();
	WMc.hIconSm = logo_icon_small;
	if (!RegisterClassExW(&WMc))
	{
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
		return 0;
	}

	HWND desktopHandle = GetDesktopWindow();
	RECT desktopRect;
	GetWindowRect(desktopHandle, &desktopRect);

	//Compute 16/9 equivalent
	LONG width; //= (desktopRect.right - desktopRect.left) / 6;
	LONG height; //= width * 9 / 16;

	float ogWidth = RECTWIDTH(desktopRect);
	float ogHeight = RECTHEIGHT(desktopRect);
#if 1
	//I always want to scale with the width
	float correctionFactor = (16.f / 9.f) * (ogHeight / ogWidth);
	width = ogWidth * correctionFactor;
	height = ogHeight;
# else
	if (ogWidth >= ogHeight) {
		float correctionFactor = (16.f / 9.f) * (ogHeight / ogWidth);
		width = ogWidth * correctionFactor;
		height = ogHeight;
	}
	else {
		float correctionFactor = (9.f / 16.f) * (ogWidth / ogHeight);
		height = ogHeight * correctionFactor;
		width = ogWidth;
	}
#endif

#define WINDOW_REDUCTION 4

#define SETTINGS_NUMBER_OF_CHECKBOXES 30.f

	//SUPERTODO(fran): I have no idea why on virtual machine with width to minimum the window starts to stretch and position is wrong

#if 0 //use screen width for manager's
	width /= WINDOW_REDUCTION;
	height /= WINDOW_REDUCTION;
#else //use the suspect checkbox technique
	//TODO(fran): what we can do is in case this values are bigger than the screen then we use the other method
	int mgr_window_width = (((float)GetSystemMetrics(SM_CXMENUCHECK))*.8f) * (SETTINGS_NUMBER_OF_CHECKBOXES*16.f/9.f);
	width = mgr_window_width;
	height = ((float)width) * 8.f / 16.f;
	//we first make it smaller to account for the fact that text will be shorter in x axis
#endif
	
#if 0 //test different sizes and aspect ratios
	width = 1024 / WINDOW_REDUCTION;//INFO(fran): remember now we are starting to use values given by windows for the specific screen, so things wont change even if the resolution does
	height = 768 / WINDOW_REDUCTION;
#endif
#undef WINDOW_REDUCTION

	int x = (RECTWIDTH(desktopRect))/2 - width/2;
	int y = (RECTHEIGHT(desktopRect)) / 2 - height / 2;
	
	//Definition of custom frame
	//INFO: All in pixels
	//SM_CXBORDER : width of window border, equivalent to SM_CXEDGE for windows with 3D look
	//SM_CXFIXEDFRAME : thickness of frame around window with caption but not sizable, is the height of the horizontal border. SM_CYFIXEDFRAME is width of vertical border
	//SM_CXEDGE : width of 3D border. SM_CYEDGE
	//SM_CXPADDEDBORDER : border padding for captioned windows
	//SM_CXSIZE : width of button in window caption
	//SM_CXSIZEFRAME : thickness of sizing border of window that can be resized, is the width of horizontal border SM_CYSIZEFRAME is the height of vertical border
	//SM_CXSMSIZE : width of small caption buttons
	//SM_CYCAPTION : height of caption area
	//SM_CYSMCAPTION : height of small caption
	FRAME.caption_height = GetBasicWindowCaptionHeight(hInstance, {x,y});//INFO: I think this is dpi aware
	if (FRAME.caption_height <= 0) {//INFO: menor igual me permite semi salvarla para multi-monitor
		//FRAME.caption_height = GetSystemMetrics(SM_CYCAPTION);
		FRAME.caption_height = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER);//INFO: this is not dpi aware
	}
	FRAME.bottom_border = 0;//We will have no borders, also be dont support resizing
	FRAME.left_border = 0;
	FRAME.right_border = 0;
	FRAME.caption_bk_brush = GetStockBrush(DKGRAY_BRUSH); //TODO(fran): take this out of FRAME and just use the one in ControlProcedures

	//More Controls' color setup
	LOGBRUSH lb;
	GetObject(FRAME.caption_bk_brush, sizeof(LOGBRUSH), &lb);
	ControlProcedures::Instance().Set_CaptionBackgroundColor(lb.lbColor);

	WindowManagerHandle = CreateWindowExW(WS_EX_TOPMOST | WS_EX_APPWINDOW, appManagerClassName.c_str(),NULL,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MINIMIZEBOX ^ WS_MAXIMIZEBOX
		,x,y,width,height,WindowHandle,nullptr,hInstance,&startup_info);
	//INFO TODO(fran): it seems that when you create a window that is bigger than the screen in some axis, that axis gets automatically reduced,
	//therefore in those cases the x and y position are actually wrong, maybe we should re-position on wm_create
	if (!WindowManagerHandle) {
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_FAIL);
		return 0;
	}
	AWT(WindowManagerHandle, SCV_LANG_MGR);
	//ShowWindow(WindowManagerHandle, nCmdShow);
	//ShowWindow(WindowManagerHandle, SW_HIDE); 
	UpdateWindow(WindowManagerHandle);

	//Settings window
	//Register Settings class
	std::wstring settings_class = L"Smart Veil Settings Franco Badenas Abal";
	WNDCLASSEXW WSc;
	WSc.cbSize = sizeof(WNDCLASSEXW);
	WSc.style = CS_HREDRAW | CS_VREDRAW;
	WSc.lpfnWndProc = WndSettingsProc;
	WSc.cbClsExtra = 0;
	WSc.cbWndExtra = 0;
	WSc.hInstance = hInstance;
	WSc.hIcon = logo_icon;
	WSc.hCursor = Cursor;
	WSc.hbrBackground = ControlProcedures::Instance().Get_BackgroundBrush();
	WSc.lpszMenuName = nullptr;
	WSc.lpszClassName = settings_class.c_str();
	WSc.hIconSm = logo_icon_small;
	if (!RegisterClassExW(&WSc))
	{
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_WINDOWCLASS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL), E_UNEXPECTED);
		return 0;
	}
	
	int settings_window_width = (GetSystemMetrics(SM_CXMENUCHECK)*.8f)*SETTINGS_NUMBER_OF_CHECKBOXES;//we first make it smaller to account for the fact that text will be shorter in x axis

	WindowSettingsHandle = CreateWindowExW(WS_EX_TOPMOST, settings_class.c_str(), NULL,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MINIMIZEBOX ^ WS_MAXIMIZEBOX
		, x, y,
#if 0
		width/2, height
#else
		settings_window_width, settings_window_width*11/9
#endif
		,WindowManagerHandle, nullptr, hInstance, &CurrentValidSettings);
	if (!WindowSettingsHandle) {
		ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_WINDOW_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), E_FAIL);
		return 0;
	}
	AWT(WindowSettingsHandle, SCV_LANG_SETTINGS);
	UpdateWindow(WindowSettingsHandle);

    DestroyCursor(Cursor);

	//Initialize Tray Icon	
	//https://docs.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-notifyicondataa
	//see https://social.msdn.microsoft.com/Forums/windows/en-US/a4d7e039-6654-4068-80b2-cd380530d92e/examples-using-win32-api-and-c-for-notification-tray-program?forum=vcgeneral
	//good example
	if (startup_info.show_tray_icon) {
		PostMessage(WindowManagerHandle, SCV_MANAGER_CREATE_TRAY, 0, 0);
	}

    ShowWindow(WindowHandle, nCmdShow);
	ShowWindow(WindowHandle, SW_MAXIMIZE);
    UpdateWindow(WindowHandle);

	//Create worker thread that will generate the image to display
	DWORD ThreadID;
	HANDLE worker_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkerThread, WindowHandle, 0, &ThreadID);
	Assert(worker_thread_handle);

    // Message loop (attempts to update screen when no other messages to process)
    MSG msg = {0};

	BOOL bRet;

	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
			Assert(0);
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//Now we have left the while loop and are ready to stop the app

    // Make sure all other threads have exited
    if (SetEvent(TerminateThreadsEvent))
    {
        ThreadMgr.WaitForThreadTermination();
    }

    // Clean up
    CloseHandle(UnexpectedErrorEvent);
    CloseHandle(ExpectedErrorEvent);
    CloseHandle(TerminateThreadsEvent);

	ReleaseMutex(single_instance_mutex);
	CloseHandle(single_instance_mutex);

	CloseHandle(worker_thread_handle);
	CloseHandle(process_next_frame_mutex);
	
	WaitForSingleObject(start_mutex, INFINITE);
	ReleaseMutex(start_mutex);
	CloseHandle(start_mutex);

	CloseHandle(thread_finished_mutex);

	if (logo_icon) DestroyIcon(logo_icon);
	if (logo_icon_small) DestroyIcon(logo_icon_small);

    if (msg.message == WM_QUIT)
    {
        // For a WM_QUIT message we should return the wParam value
		
		//Delete the tray icon
		//if(startup_info.show_tray_icon) Shell_NotifyIcon(NIM_DELETE, &notification);
		//TRAY_HANDLER::Instance().DestroyTrayIcon(WindowManagerHandle, 1);
		return static_cast<INT>(msg.wParam);
    }

    return 0;
}

//
// Shows help
//
//void ShowHelp()
//{
//    DisplayMsg(L"The following optional parameters can be used -\n  /output [all | n]\t\tto duplicate all outputs or the nth output\n  /?\t\t\tto display this help section",
//               L"Proper usage", S_OK);
//}

//
// Process command line parameters
//
bool ProcessCmdline(_Out_ INT* Output)
{
    *Output = -1;

    // __argv and __argc are global vars set by system
    for (UINT i = 1; i < static_cast<UINT>(__argc); ++i)
    {
        if ((strcmp(__argv[i], "-output") == 0) ||
            (strcmp(__argv[i], "/output") == 0))
        {
            if (++i >= static_cast<UINT>(__argc))
            {
                return false;
            }

            if (strcmp(__argv[i], "all") == 0)
            {
                *Output = -1;
            }
            else
            {
                *Output = atoi(__argv[i]);
            }
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}

void SetupSettings(HWND hwnd,HINSTANCE hInstance, SETTINGS* current_settings) {
	//Settings needs:
	//·language (combobox)
	//·reduce dangerous slider values (checkbox)
	//·remember manager pos (checkbox)
	//·show manager on app startup (checkbox)
	//·show tooltips (checkbox)
	//·show tray icon (checkbox)
	//·show veil on startup (combobox)
	//·start with windows (checkbox)
	//·hotkey (hotkey), hotkey at the end since it's the most probable that will change, it will be closer to the save button
	//·save (button), if successful save then close settings, if error eg bad hotkey then leave it open and tell the problem

	//TODO(fran): checkbox to let the user decide what do left & right click on tray icon do?

	//TODO(fran): we need a hidden control with the font already set so we can test the size of text and adjust automatically


	RECT rec;
	GetMyClientRect(hwnd, &rec);

	float width = RECTWIDTH(rec);
	float height = RECTHEIGHT(rec);

	float paddingX = rec.left + width * .05f;
	float paddingY = height * .01f;
	float addPaddingY = paddingY;
	paddingY += rec.top;
	
	float TextY = height * .08f;//default text height

	POINT checkbox_text;
	checkbox_text.x = width * .9f;
	checkbox_text.y = TextY;
	HWND manager_on_startup = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_MANAGER_ON_STARTUP, hInstance, NULL);
	AWT(manager_on_startup, SCV_LANG_SETTINGS_SHOW_MGR);
	SetWindowLongPtr(manager_on_startup, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(manager_on_startup, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
	
	SendMessageW(manager_on_startup, BM_SETCHECK, current_settings->show_manager_on_startup? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	HWND show_tray = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_SHOW_TRAY, hInstance, NULL);
	AWT(show_tray, SCV_LANG_SETTINGS_SHOW_TRAY);
	SetWindowLongPtr(show_tray, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(show_tray, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(show_tray, BM_SETCHECK, current_settings->show_tray_icon ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	//TODO(fran): move this guy below, after we already said the on startup == on application startup
	HWND dangerous_slider = CreateWindowW(L"Button", NULL, WS_VISIBLE| WS_CHILD| BS_AUTOCHECKBOX
	, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_DANGEROUS_SLIDER, hInstance, NULL);
	AWT(dangerous_slider, SCV_LANG_SETTINGS_REDUCE_SLIDER);
	SetWindowLongPtr(dangerous_slider, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(dangerous_slider, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(dangerous_slider, BM_SETCHECK, current_settings->reduce_dangerous_slider_values ? BST_CHECKED : BST_UNCHECKED, 0);
	
	//INFO: See displaying text in https://docs.microsoft.com/en-us/windows/win32/controls/tooltip-controls
	TOOLTIP_REPO::Instance().CreateToolTip(SCV_SETTINGS_DANGEROUS_SLIDER, hwnd, SCV_LANG_SETTINGS_REDUCE_SLIDER_TIP);

	paddingY += checkbox_text.y + addPaddingY;

	HWND remember_manager_pos = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_MANAGER_POS, hInstance, NULL);
	AWT(remember_manager_pos, SCV_LANG_SETTINGS_REMEMBER_MGR_POS);
	SetWindowLongPtr(remember_manager_pos, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(remember_manager_pos, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(remember_manager_pos, BM_SETCHECK, current_settings->remember_manager_position ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	HWND show_tooltips = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_SHOW_TOOLTIPS, hInstance, NULL);
	AWT(show_tooltips, SCV_LANG_SETTINGS_SHOW_TOOLTIP);
	SetWindowLongPtr(show_tooltips, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(show_tooltips, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(show_tooltips, BM_SETCHECK, current_settings->show_tooltips ? BST_CHECKED : BST_UNCHECKED, 0);

	TOOLTIP_REPO::Instance().CreateToolTip(SCV_SETTINGS_SHOW_TOOLTIPS, hwnd, SCV_LANG_SETTINGS_SHOW_TOOLTIP_TIP);
	
	paddingY += checkbox_text.y + addPaddingY;

	HWND start_with_windows = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_START_WITH_WINDOWS, hInstance, NULL);
	AWT(start_with_windows, SCV_LANG_SETTINGS_START_WITH_WIN);
	SetWindowLongPtr(start_with_windows, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(start_with_windows, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(start_with_windows, BM_SETCHECK, current_settings->start_with_windows ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	POINT veil_on_startup_text;
	veil_on_startup_text.x = width * .50f;
	veil_on_startup_text.y = TextY;
	//TODO(fran): proper static control text alignment
	HWND veil_on_startup = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE
		, paddingX, paddingY, veil_on_startup_text.x, veil_on_startup_text.y, hwnd, (HMENU)SCV_SETTINGS_VEIL_STARTUP_TITLE, NULL, NULL);
	AWT(veil_on_startup, SCV_LANG_SETTINGS_TURN_ON);

	POINT combobox_text;
	combobox_text.x = width * .315f;
	combobox_text.y = veil_on_startup_text.y;

	HWND veil_on_startup_combo = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP
		, paddingX + veil_on_startup_text.x*1.05f, paddingY, combobox_text.x, combobox_text.y, hwnd, (HMENU)SCV_SETTINGS_VEIL_STARTUP_COMBO, hInstance, NULL);

	ACT(veil_on_startup_combo, VEIL_ON_STARTUP::YES, SCV_LANG_SETTINGS_TURN_ON_YES);//INFO(fran):this values MUST be lined up with LANGUAGE enum
	ACT(veil_on_startup_combo, VEIL_ON_STARTUP::NO, SCV_LANG_SETTINGS_TURN_ON_NO);
	ACT(veil_on_startup_combo, VEIL_ON_STARTUP::REMEMBER_LAST_STATE, SCV_LANG_SETTINGS_TURN_ON_REMEMBER);
	SendMessageW(veil_on_startup_combo, CB_SETCURSEL, current_settings->show_veil_on_startup, 0);
	SetWindowLongPtr(veil_on_startup_combo, GWL_USERDATA, COMBO_ICON);
	SetWindowSubclass(veil_on_startup_combo, ControlProcedures::Instance().ComboProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	paddingY += combobox_text.y + addPaddingY;

	POINT language_text;
	language_text.x = width * .25f;
	language_text.y = TextY;
	HWND LanguageText = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE
		, paddingX, paddingY, language_text.x, language_text.y, hwnd, (HMENU)SCV_SETTINGS_LANG_TITLE, NULL, NULL);
	AWT(LanguageText, SCV_LANG_SETTINGS_LANG);

	HWND language_combo = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP
		, paddingX + language_text.x*1.1f, paddingY, combobox_text.x, combobox_text.y, hwnd, (HMENU)SCV_SETTINGS_LANG_COMBO, hInstance, NULL);

	SendMessageW(language_combo, CB_ADDSTRING, LANGUAGE_MANAGER::LANGUAGE::ENGLISH, (LPARAM)L"English");//INFO(fran):this values MUST be lined up with LANGUAGE enum
	SendMessageW(language_combo, CB_ADDSTRING, LANGUAGE_MANAGER::LANGUAGE::SPANISH, (LPARAM)L"Español");//INFO: this strings will not change with langs, each lang will be written like it should
	SendMessageW(language_combo, CB_SETCURSEL, current_settings->language, 0);
	SetWindowLongPtr(language_combo, GWL_USERDATA, COMBO_ICON);
	SetWindowSubclass(language_combo, ControlProcedures::Instance().ComboProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	paddingY += combobox_text.y + addPaddingY;

	//TODO(fran): will this be a shortcut to turn the veil on and off, or to open smart veil manager??
	//			  or do I make two shortcuts
	HWND Hotkey_text = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE
		, paddingX, paddingY, checkbox_text.x, checkbox_text.y, hwnd, NULL, NULL, NULL);
	AWT(Hotkey_text, SCV_LANG_SETTINGS_SHORTCUT);
	paddingY += checkbox_text.y + addPaddingY;

	// Ensure that the common control DLL is loaded. 
	INITCOMMONCONTROLSEX icex; // declare an INITCOMMONCONTROLSEX Structure
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_HOTKEY_CLASS; // set dwICC member to ICC_HOTKEY_CLASS    
	InitCommonControlsEx(&icex); // this loads the Hot Key control class.

	POINT hotkey_value_text;
	hotkey_value_text.x = width * .6f;
	hotkey_value_text.y = TextY;
	HWND HotkeyValue = CreateWindowEx(0, HOTKEY_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE
		, paddingX, paddingY, hotkey_value_text.x, hotkey_value_text.y, hwnd, (HMENU)SCV_SETTINGS_HOTKEY, NULL, NULL);
	//TODO(fran): test whether the hotkey string will change automatically depending on the lang or how to do it
	paddingY += hotkey_value_text.y + addPaddingY;

	SetWindowSubclass(HotkeyValue, ControlProcedures::Instance().HotkeyProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	// Set rules for invalid key combinations. If the user does not supply a
	// modifier key, use ALT as a modifier. If the user supplies SHIFT as a 
	// modifier key, use SHIFT + ALT instead.
	SendMessage(HotkeyValue,
		HKM_SETRULES,
		(WPARAM)HKCOMB_NONE,   // invalid key combinations 
		MAKELPARAM(HOTKEYF_ALT, 0));       // add ALT to invalid entries 

#if 0
	SendMessage(HotkeyValue,
		HKM_SETRULES,
		(WPARAM)HKCOMB_S,   // invalid key combinations 
		MAKELPARAM(HOTKEYF_SHIFT | HOTKEYF_ALT, 0));
#endif
	//Register hotkey
	//TODO(fran): this shouldnt be here
	if (CurrentValidHotkeyModifiers || CurrentValidHotkeyVirtualKey) { //if we have some modifiers and a key then try to register
		if (!RegisterHotKey(GetWindow(hwnd, GW_OWNER), 0, CurrentValidHotkeyModifiers | MOD_NOREPEAT, CurrentValidHotkeyVirtualKey)) {
			SendMessageW(HotkeyValue, SCV_HOTKEY_REG_FAILED, 0, 0);
			//Convert virtual key+modifier into string
			//http://cottonvibes.blogspot.com/2010/11/virtual-key-code-to-string.html
			std::wstring HotkeyStr = HotkeyString(CurrentValidHotkeyModifiers, CurrentValidHotkeyVirtualKey);
			std::wstring hotname = RS(SCV_LANG_SETTINGS_SHORTCUT_REG_FAIL_1);
			hotname+= L" " + HotkeyStr + L"\n" + RS(SCV_LANG_SETTINGS_SHORTCUT_REG_FAIL_2);
			MessageBoxW(GetWindow(hwnd, GW_OWNER), hotname.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), MB_OK | MB_ICONEXCLAMATION);
		}
		else {
			SendMessageW(HotkeyValue, SCV_HOTKEY_REG_SUCCESS, 0, 0);
		}

		//TODO(fran): if hotkey registration fails and the vk and modifs are not 0 then send to hotkeycontrol the values anyway,
		//			  maybe we could paint the text red to indicate there is a problem and needs changing, but that way we dont delete
		//			  what they already wrote and might forget

		// Set the default hot key for this window. 
		SendMessage(HotkeyValue,
			HKM_SETHOTKEY,
			MAKEWORD(CurrentValidHotkeyVirtualKey, HotkeyModifiersToHotkeyControlModifiers(CurrentValidHotkeyModifiers)),
			0);
	}

	//TODO(fran): do we make this the floppy icon?
	POINT save_text;
	save_text.x = width * .25f;
	save_text.y = TextY;
	HWND Save= CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW //| BS_NOTIFY
		, rec.left + width - save_text.x - paddingX, paddingY, save_text.x, save_text.y, hwnd, (HMENU)SCV_SETTINGS_SAVE, NULL, NULL);
	AWT(Save, SCV_LANG_SETTINGS_SAVE);

	paddingY += save_text.y + addPaddingY;

	SetWindowSubclass(Save, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//Basic update counter
	POINT UpdateCounterSize = { MS_UPDATE? (LONG)(width * .2f) : (LONG)(width * .1f) ,(LONG)(height * .08f) };
	HWND UpdateCounter = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD
		, rec.left, rec.top + height-UpdateCounterSize.y, UpdateCounterSize.x, UpdateCounterSize.y, hwnd, (HMENU)SCV_UPDATE_COUNTER_TEXT, NULL, NULL);

	POINT UpdateCounterUnitSize = { (LONG)(width *.08f),(LONG)(height*.1f) };
	HWND UpdateCounterUnit = CreateWindowW(L"Static", MS_UPDATE? L"ms" : L"fps", WS_VISIBLE | WS_CHILD
		, rec.left + UpdateCounterSize.x, rec.top + height - UpdateCounterSize.y, UpdateCounterUnitSize.x, UpdateCounterUnitSize.y, hwnd, NULL, NULL, NULL);

	//Font set-up
	HFONT font;

	LOGFONTW lf;

	memset(&lf, 0, sizeof(lf));
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfHeight = (LONG)(-width * .046f);
	//@ver: lf.pszFaceName y EnumFontFamiliesW
	font = CreateFontIndirectW(&lf);

	if (font == NULL)
	{
		MessageBox(hwnd, RCS(SCV_LANG_ERROR_FONT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), MB_OK);
	}
	else {
		SendMessageW(LanguageText, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(language_combo, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(dangerous_slider, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(remember_manager_pos, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(manager_on_startup, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(show_tooltips, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(show_tray, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(veil_on_startup, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(veil_on_startup_combo, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(start_with_windows, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(Hotkey_text, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(HotkeyValue, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(Save, WM_SETFONT, (WPARAM)font, TRUE);

		SendMessageW(UpdateCounter, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(UpdateCounterUnit, WM_SETFONT, (WPARAM)font, TRUE);
	}
}

inline void SaveCurrentValidSettings(HWND settings_window, SETTINGS &current_settings) {//TODO(fran): should this and the restore procedure be in startup_and_settings.cpp ? hard to decouple
	//We ask each control for its current state and save it to current_settings
	
	//current_settings.hotkey_modifiers = CurrentValidHotkeyModifiers; //we ask currentvalidhotkey since it may or may not be modified depending on the validity of the hotkey
	//current_settings.hotkey_virtual_key = CurrentValidHotkeyVirtualKey;

	current_settings.language = (LANGUAGE_MANAGER::LANGUAGE)SendDlgItemMessage(WindowSettingsHandle, SCV_SETTINGS_LANG_COMBO, CB_GETCURSEL, 0, 0);
	current_settings.show_veil_on_startup = (VEIL_ON_STARTUP)SendDlgItemMessage(WindowSettingsHandle, SCV_SETTINGS_VEIL_STARTUP_COMBO, CB_GETCURSEL, 0, 0);

	current_settings.reduce_dangerous_slider_values = isChecked(GetDlgItem(WindowSettingsHandle, SCV_SETTINGS_DANGEROUS_SLIDER));
	current_settings.remember_manager_position = isChecked(GetDlgItem(WindowSettingsHandle, SCV_SETTINGS_MANAGER_POS));
	current_settings.show_manager_on_startup = isChecked(GetDlgItem(WindowSettingsHandle, SCV_SETTINGS_MANAGER_ON_STARTUP));
	current_settings.show_tooltips = isChecked(GetDlgItem(WindowSettingsHandle, SCV_SETTINGS_SHOW_TOOLTIPS));
	current_settings.show_tray_icon = isChecked(GetDlgItem(WindowSettingsHandle, SCV_SETTINGS_SHOW_TRAY));
	current_settings.start_with_windows = isChecked(GetDlgItem(WindowSettingsHandle, SCV_SETTINGS_START_WITH_WINDOWS));

}

void RestoreCurrentValidSettingsToControls(HWND settings_window, SETTINGS const& current_settings) {
	//Resets the state of the controls to the one that is saved in current_settings
	
	//If what is written in the hotkey is different from what currentvalidmod/vk has then reset to the valid ones (if not null)

	WORD hotkey_on_control = SendDlgItemMessage(settings_window, SCV_SETTINGS_HOTKEY, HKM_GETHOTKEY, 0, 0);
	BYTE modifs_on_control = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(hotkey_on_control));
	BYTE vk_on_control = LOBYTE(hotkey_on_control);
	
	if (CurrentValidHotkeyModifiers || CurrentValidHotkeyVirtualKey) {
		if(CurrentValidHotkeyModifiers!= modifs_on_control || CurrentValidHotkeyVirtualKey != vk_on_control)
			SendDlgItemMessage(settings_window, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_RESET_TEXT, (WPARAM)CurrentValidHotkeyVirtualKey, (LPARAM)CurrentValidHotkeyModifiers);
	}

	//TODO(fran): what am I gonna do with the hotkey?
	SendDlgItemMessage(settings_window, SCV_SETTINGS_LANG_COMBO, CB_SETCURSEL, current_settings.language, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_VEIL_STARTUP_COMBO, CB_SETCURSEL, current_settings.show_veil_on_startup, 0);

	SendDlgItemMessage(settings_window, SCV_SETTINGS_DANGEROUS_SLIDER, BM_SETCHECK, current_settings.reduce_dangerous_slider_values? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_MANAGER_POS, BM_SETCHECK, current_settings.remember_manager_position ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_MANAGER_ON_STARTUP, BM_SETCHECK, current_settings.show_manager_on_startup ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_SHOW_TOOLTIPS, BM_SETCHECK, current_settings.show_tooltips ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_SHOW_TRAY, BM_SETCHECK, current_settings.show_tray_icon ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_START_WITH_WINDOWS, BM_SETCHECK, current_settings.start_with_windows ? BST_CHECKED : BST_UNCHECKED, 0);
}

void Settings_SaveHotkey(HWND mgr,HWND settings) {
	WORD new_hotkey = SendDlgItemMessage(settings,SCV_SETTINGS_HOTKEY, HKM_GETHOTKEY, 0, 0);
	BYTE new_modifs = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(new_hotkey));
	BYTE new_vk = LOBYTE(new_hotkey);
	if (new_hotkey) { //cant check if both vk and modif exist cause there are vks that are accepted alone
		if (new_modifs != CurrentValidHotkeyModifiers || new_vk != CurrentValidHotkeyVirtualKey) {

			//+THIS IS NEW
			if (CurrentValidHotkeyVirtualKey || CurrentValidHotkeyModifiers) {
				UnregisterHotKey(mgr, 0); //INFO: unregister previous hotkey in case there was any, this has to be done only cause of the stupid hotkey control, the new
											// hotkey replaces the old one
			}
			//+

			if (RegisterHotKey(mgr, 0, new_modifs | MOD_NOREPEAT, new_vk)) {
				//Hotkey registration succeded, we have a new valid hotkey
				SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_REG_SUCCESS, 0, 0);
				CurrentValidHotkeyVirtualKey = new_vk;
				CurrentValidHotkeyModifiers = new_modifs;

				//TODO(fran): we still need to unregister the hotkey cause the stupid control still thinks the old one is valid and doesnt let
				// you write it
				//UnregisterHotKey(mgr, 0);//I think this unregisters only the older hotkey
				//INFO(fran): it seems like we dont need to unregister the previous hotkey, did i misunderstand the docs?

				//TODO(fran): we could disable the button to let the user understand the changes were applied successfully,
				// right now it is not clear wheter it worked or not
			}
			else {

				//+THIS IS NEW
				//Re-register the previous hotkey
				if (CurrentValidHotkeyVirtualKey || CurrentValidHotkeyModifiers) { //TODO(fran): check this doesnt introduce any bugs
					if (!RegisterHotKey(mgr, 0, CurrentValidHotkeyModifiers | MOD_NOREPEAT, CurrentValidHotkeyVirtualKey)) {
						CurrentValidHotkeyVirtualKey = NULL;
						CurrentValidHotkeyModifiers = NULL;
						//Clear text for hotkey control
						SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, HKM_SETHOTKEY, 0, 0);
						//Error msg to the user telling both the new and previous hotkey failed, and we need a new one
						MessageBox(settings, RCS(SCV_LANG_ERROR_OLD_AND_NEW_HOTKEYS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
					}
				}
				//+

				//Hotkey registration failed
				SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_REG_FAILED, 0, 0);

				//TODO(fran): if failed write the currently valid hotkey instead of red color with bad hotkey ? 
			}
		}
	}
}

LRESULT CALLBACK WndSettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int numberOfUpdates = 0;

	switch (message) {
	case WM_CREATE:
	{
		SetWindowTheme(hWnd, L"", L"");//INFO: to avoid top curved corners on frame
		CREATESTRUCT* creation_info = (CREATESTRUCT*)lParam;
		SETTINGS* current_settings = (SETTINGS*)creation_info->lpCreateParams;
		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE);
		SetupSettings(hWnd,hInstance, current_settings);
	
		//Create procedure to add things to the "non client" area
		RECT rc;
		GetWindowRect(hWnd, &rc);

		float button_height = FRAME.caption_height;
		float button_width = button_height * 16.f / 9.f;

		HWND close_button = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | BS_ICON
			, RECTWIDTH(rc) - button_width - FRAME.right_border, 0, button_width, button_height, hWnd, (HMENU)SCV_CUSTOMFRAME_CLOSE, hInstance, NULL);
		SetWindowLongPtr(close_button, GWL_USERDATA, CROSS_ICON);
		SetWindowSubclass(close_button, ControlProcedures::Instance().CaptionButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
		TOOLTIP_REPO::Instance().CreateToolTip(SCV_CUSTOMFRAME_CLOSE, hWnd, SCV_LANG_CLOSE);
		//

		//TEST
		SetTimer(hWnd, 1, 1000, NULL);
		
		break;
	}
	case WM_PAINT:
	{
		return PaintCaption(hWnd);
	}
	case WM_NCACTIVATE://stuff for custom frame
	{
		return TRUE;
	}
	case WM_NCHITTEST://stuff for custom frame
	{
		return HitTestNCA(hWnd, wParam, lParam);
	}
	case WM_NCCALCSIZE://stuff for custom frame
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
	case WM_NCPAINT://stuff for custom frame
	{
		return 0;
	}
	case WM_TIMER: {
		KillTimer(hWnd, 1);
		SetWindowText(GetDlgItem(hWnd, SCV_UPDATE_COUNTER_TEXT), std::to_wstring(numberOfUpdates).c_str());
		numberOfUpdates = 0;
		SetTimer(hWnd, 1, 1000, NULL);
		break;
	}
	case SCV_UPDATE_COUNTER:
	{
#if MS_UPDATE
		double val = *(double*)wParam;
		SetWindowText(GetDlgItem(hWnd,SCV_UPDATE_COUNTER_TEXT), std::to_wstring(val).c_str());
#else 
		numberOfUpdates++;
#endif
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case SCV_CUSTOMFRAME_CLOSE:
		{
			//CloseWindow(hWnd);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		case SCV_SETTINGS_SAVE:
		{
			SETTINGS previous_settings = CurrentValidSettings;

			//TODO(fran): some system for executing the new changes
			//first check what changed, and based on that generate a list of actions to execute
			Settings_SaveHotkey(GetWindow(hWnd, GW_OWNER),hWnd);
			SaveCurrentValidSettings(hWnd, CurrentValidSettings);

			if(previous_settings.show_tooltips != CurrentValidSettings.show_tooltips)
				TOOLTIP_REPO::Instance().ActivateTooltips(CurrentValidSettings.show_tooltips);
			if (previous_settings.show_tray_icon != CurrentValidSettings.show_tray_icon) {
				if (CurrentValidSettings.show_tray_icon) SendMessage(GetWindow(hWnd, GW_OWNER), SCV_MANAGER_CREATE_TRAY, 0, 0);
				else SendMessage(GetWindow(hWnd, GW_OWNER), SCV_MANAGER_REMOVE_TRAY, 0, 0);
			}
			if (previous_settings.language != CurrentValidSettings.language) {
				LCID ret = LANGUAGE_MANAGER::Instance().ChangeLanguage(CurrentValidSettings.language);
				TRAY_HANDLER::Instance().UpdateTrayTips();
			}

			if (previous_settings.start_with_windows != CurrentValidSettings.start_with_windows) {
				if (CurrentValidSettings.start_with_windows) {
					LSTATUS reg_res = RegisterProgramOnStartup(GetExePath());
				}
				else {
					LSTATUS unreg_res = UnregisterProgramFromStartup();
				}
			}

			//TODO(fran): if the new executed changes, ie hotkey, all work then hide the window, otherwise show error msg and dont hide
			// I think sendmessage returns a value, so I can still check if everything worked, even through there

			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_CTLCOLORLISTBOX:
	{
		HDC comboboxDC = (HDC)wParam;
		SetTextColor(comboboxDC, ControlProcedures::Instance().Get_HighlightColor());
		SetBkColor(comboboxDC, ControlProcedures::Instance().Get_BackgroundColor());
		
		return (INT_PTR)ControlProcedures::Instance().Get_BackgroundBrush();
	}
	case WM_CTLCOLORSTATIC:
	{
		HDC staticDC = (HDC)wParam;
		SetTextColor(staticDC, ControlProcedures::Instance().Get_HighlightColor());
		SetBkColor(staticDC, ControlProcedures::Instance().Get_BackgroundColor());
		return (INT_PTR)ControlProcedures::Instance().Get_BackgroundBrush();
	}
	//case WM_CTLCOLORBTN: {}
	//case WM_DRAWITEM: { //INFO: I'm using it only to draw buttons

	//	return DrawButton((DRAWITEMSTRUCT*)lParam, wParam);
	//}
	case WM_CLOSE:
	{
		ShowWindow(hWnd, SW_HIDE);
		RestoreCurrentValidSettingsToControls(hWnd, CurrentValidSettings);

		break;
	}
	default: 
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

inline double GetPCFrequency() {
	LARGE_INTEGER li;
	Assert(QueryPerformanceFrequency(&li));
	return double(li.QuadPart) / 1000.0; //milliseconds
}

inline void StartCounter(__int64 &CounterStart) {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

inline double GetCounter(__int64 CounterStart, double PCFreq)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

//*Everything related to updating the contents of the veil takes place here
DWORD WINAPI WorkerThread(LPVOID Param) {
	HWND veil = (HWND)Param;//TODO(fran): every important parameter should be put into a struct and passed through this
	DYNAMIC_WAIT DynamicWait;
	bool FirstTime = true;
	DUPL_RETURN Ret;
	DWORD mut_ret = WaitForSingleObject(start_mutex, INFINITE);//TODO(fran): check we got a valid return value
	Assert(mut_ret == WAIT_OBJECT_0);
	ReleaseMutex(start_mutex);
	//Now we know the HWND is properly initialized and we can start working with it

	mut_ret = WaitForSingleObject(thread_finished_mutex, INFINITE);//Mutex that will be released when the thread finishes
	Assert(mut_ret == WAIT_OBJECT_0);

	//Basic update rate checking system https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter
	double PCFreq = GetPCFrequency();
	__int64 CounterStart = 0;
	double CounterVal;

	while (WaitForSingleObject(process_next_frame_mutex, INFINITE)== WAIT_OBJECT_0) {
		
		if (ProgramFinished) {//TODO(fran): this is a bit botched
			//TODO(fran): cleanup of the other threads?
			ReleaseMutex(process_next_frame_mutex);
			ReleaseMutex(thread_finished_mutex);
			return 0;
		}

		StartCounter(CounterStart);
		
		Ret = DUPL_RETURN_SUCCESS;
		if (WaitForSingleObjectEx(UnexpectedErrorEvent, 0, FALSE) == WAIT_OBJECT_0)
		{
			// Unexpected error occurred so exit the application
			ReleaseMutex(process_next_frame_mutex);
			ReleaseMutex(thread_finished_mutex);
			std::wstring error_msg = RS(SCV_LANG_ERROR_UNEXPECTED);
			error_msg += L"\n";
			error_msg += RS(SCV_LANG_ERROR_CLOSING_APP);
			MessageBoxW(NULL, error_msg.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
			DestroyWindow(veil);
			return 1;
		}
		else if (FirstTime || WaitForSingleObjectEx(ExpectedErrorEvent, 0, FALSE) == WAIT_OBJECT_0)
		{
			//All thread setup for execution runs here, I think
			if (!FirstTime)
			{
				// Terminate other threads
				SetEvent(TerminateThreadsEvent);
				ThreadMgr.WaitForThreadTermination();
				ResetEvent(TerminateThreadsEvent);
				ResetEvent(ExpectedErrorEvent);

				// Clean up
				ThreadMgr.Clean();
				OutMgr.CleanRefs();

				// As we have encountered an error due to a system transition we wait before trying again, using this dynamic wait
				// the wait periods will get progressively long to avoid wasting too much system resource if this state lasts a long time
				DynamicWait.Wait();
			}
			else
			{
				// First time through the loop so nothing to clean up
				FirstTime = false;
			}

			// Re-initialize
			Ret = OutMgr.InitOutput(WindowHandle, SingleOutput, &OutputCount, &DeskBounds);
			if (Ret == DUPL_RETURN_SUCCESS)
			{
				HANDLE SharedHandle = OutMgr.GetSharedHandle();
				if (SharedHandle)
				{
					Ret = ThreadMgr.Initialize(SingleOutput, OutputCount, UnexpectedErrorEvent, ExpectedErrorEvent, TerminateThreadsEvent, SharedHandle, &DeskBounds);
				}
				else
				{
					DisplayMsg(RCS(SCV_LANG_ERROR_SHARED_SURF_HANDLE_GET), RCS(SCV_LANG_ERROR_SMARTVEIL), S_OK);
					Ret = DUPL_RETURN_ERROR_UNEXPECTED;
				}
			}

		}

		//TODO(fran): could it be that some of the flickering is produced cause we havent yet updated our current surface?
		// if that were happening then we would be presenting the one from two frames ago

		// Nothing else to do, so try to present to write out to window
		Ret = OutMgr.UpdateApplicationWindow(ThreadMgr.GetPointerInfo());

		CounterVal = GetCounter(CounterStart, PCFreq);
		PostMessage(WindowSettingsHandle, SCV_UPDATE_COUNTER , (WPARAM)&CounterVal,0);//this is clearly not great cause we dont know when we are gonna read this value

		// Check if for errors
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			if (Ret == DUPL_RETURN_ERROR_EXPECTED)
			{
				// Some type of system transition is occurring so retry
				SetEvent(ExpectedErrorEvent);
			}
			else
			{
				// Unexpected error so exit
				ReleaseMutex(process_next_frame_mutex);
				ReleaseMutex(thread_finished_mutex);
				std::wstring error_msg = RS(SCV_LANG_ERROR_UNEXPECTED);
				error_msg += L"\n";
				error_msg += RS(SCV_LANG_ERROR_CLOSING_APP);
				MessageBoxW(NULL, error_msg.c_str(), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
				DestroyWindow(veil);
				return 1;
			}
		}
		ReleaseMutex(process_next_frame_mutex);

#if LIMIT_UPDATE
		if (CounterVal < TargetMs) 
			Sleep(max(TargetMs - CounterVal,0));
#endif
	}

	ReleaseMutex(thread_finished_mutex);
	return 0;
}

//
// Window message processor
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
	{
		LRESULT res = DefWindowProc(hWnd, message, wParam, lParam);
		ReleaseMutex(start_mutex);//tells the other thread that it can start working with this HWND
		return res;
	}
	case SCV_VEIL_SHOW_MGR://Only used in case of multiple instances so the new one can tell the one already running to show the manager
	{
		ShowWindow(WindowManagerHandle, SW_SHOW);
		break;
	}
    case WM_DESTROY:
    {
		//Wait for image processing thread to exit
		ProgramFinished = TRUE;
		if (!isTurnedOn)ReleaseMutex(process_next_frame_mutex);
		WaitForSingleObject(thread_finished_mutex, INFINITE);
		ReleaseMutex(thread_finished_mutex);

        PostQuitMessage(0);
        break;
    }
    case WM_SIZE:
    {
        // Tell output manager that window size has changed
        OutMgr.WindowResize();
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HWND ThresholdSlider;
HWND OpacitySlider;
HWND ThresholdPos;
HWND OpacityPos;

HWND TurnOnOff;

std::wstring first_numberstring(std::wstring const & str)
{
https://stackoverflow.com/questions/30073839/c-extract-number-from-the-middle-of-a-string
	std::size_t const n = str.find_first_of(L"0123456789");
	if (n != std::wstring::npos)
	{
		std::size_t const m = str.find_first_not_of(L"0123456789", n);
		return str.substr(n, m != std::string::npos ? m - n : m);
	}
	return std::wstring();
}

void SetupMgr(HWND hWnd,HINSTANCE hInstance, const STARTUP_INFO* startup_info) {
	RECT windowRect;
	GetMyClientRect(hWnd,&windowRect);
	
	float width = RECTWIDTH(windowRect);
	float height = RECTHEIGHT(windowRect);

	float paddingX = windowRect.left + width * .1f; //at .1 objects will start 10% to the side of the window
	float paddingY = height*.114f; //at .1 there will be less than 10 objects for sure
	float addPaddingY = paddingY;
	paddingY += windowRect.top;

	float ThreeQuarterWindowX = width*.5f + windowRect.left;

	float ThresholdTextX = width * .225f;
	float TextY = height * .125f;
	HWND ThresholdText = CreateWindowW(L"Static",NULL, WS_VISIBLE | WS_CHILD
		, paddingX, paddingY, ThresholdTextX, TextY,hWnd,(HMENU)SCV_THRESHOLD_TITLE,NULL,NULL);
	AWT(ThresholdText, SCV_LANG_MGR_THRESHOLD);

	TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, ThresholdText, SCV_LANG_MGR_THRESHOLD_TIP);

	float SliderTextX = width*.06f;
	ThresholdPos = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD
		, paddingX + ThresholdTextX, paddingY, SliderTextX, TextY, hWnd, NULL, NULL, NULL);

	TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, ThresholdPos, SCV_LANG_MGR_THRESHOLD_TIP);

	float PercentTextX = width * .05f;
	HWND ThresholdPercentText = CreateWindowW(L"Static", L"%", WS_VISIBLE | WS_CHILD
		, paddingX+ ThresholdTextX + SliderTextX, paddingY, PercentTextX, TextY, hWnd, NULL, NULL, NULL);

	paddingY += TextY *1.8f;

	TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, ThresholdPercentText, SCV_LANG_MGR_THRESHOLD_TIP);

	float SliderX = width * .3f;
	float SliderY = height * .091f;
	ThresholdSlider = CreateWindowExW(0, TRACKBAR_CLASS, 0, WS_CHILD| WS_VISIBLE | TBS_NOTICKS
		, paddingX, paddingY, SliderX, SliderY, hWnd, (HMENU)SCV_TOOLTIP_THRESHOLD_SLIDER, NULL, NULL);

	paddingY += addPaddingY+ SliderY;

	TOOLTIP_REPO::Instance().CreateToolTip(SCV_TOOLTIP_THRESHOLD_SLIDER, hWnd, SCV_LANG_MGR_THRESHOLD_TIP);

	SendMessage(ThresholdSlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 99));
	SendMessage(ThresholdSlider, TBM_SETPAGESIZE, 0, (LPARAM)5);
	SendMessage(ThresholdSlider, TBM_SETLINESIZE, 0, (LPARAM)5);
	//TODO(fran): change the amount the trackbar moves when the mousewheel is scrolled
	//·https://www.purebasic.fr/english/viewtopic.php?t=38749
	SendMessage(ThresholdSlider, TBM_SETPOS,(WPARAM)TRUE, (LPARAM)startup_info->slider_threshold_pos);


	//float OpacityTextX = width * .165f;
	HWND OpacityText = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD
		, paddingX, paddingY, ThresholdTextX, TextY, hWnd, NULL, NULL, NULL);
	AWT(OpacityText, SCV_LANG_MGR_OPACITY);

	TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, OpacityText, SCV_LANG_MGR_OPACITY_TIP);

	OpacityPos = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD
		, paddingX + ThresholdTextX, paddingY, SliderTextX, TextY, hWnd, NULL, NULL, NULL);

	TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, OpacityPos, SCV_LANG_MGR_OPACITY_TIP);

	HWND OpacityPercentText = CreateWindowW(L"Static", L"%", WS_VISIBLE | WS_CHILD
		, paddingX + ThresholdTextX + SliderTextX, paddingY, PercentTextX, TextY, hWnd, NULL, NULL, NULL);

	TOOLTIP_REPO::Instance().CreateToolTipForRect(hWnd, OpacityPercentText, SCV_LANG_MGR_OPACITY_TIP);

	paddingY += TextY * 1.8f;

	OpacitySlider = CreateWindowExW(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_NOTICKS
		, paddingX, paddingY, SliderX, SliderY, hWnd, (HMENU)SCV_TOOLTIP_OPACITY_SLIDER, NULL, NULL);

	TOOLTIP_REPO::Instance().CreateToolTip(SCV_TOOLTIP_OPACITY_SLIDER, hWnd, SCV_LANG_MGR_OPACITY_TIP);

	paddingY += addPaddingY + SliderY;

	SendMessage(OpacitySlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 99));
	SendMessage(OpacitySlider, TBM_SETPAGESIZE, 0, (LPARAM)5);
	SendMessage(OpacitySlider, TBM_SETLINESIZE, 0, (LPARAM)5);
	SendMessage(OpacitySlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)startup_info->slider_opacity_pos);

	WCHAR textPos[4];
	float pos = (float)SendMessage(ThresholdSlider, TBM_GETPOS, 0, 0);
	_itow(pos, textPos, 10);
	SetWindowTextW(ThresholdPos, textPos);
	OutMgr.SetThreshold(pos / 100.f); //TODO(fran): the setup for the outputmgr should go in a separate function

	pos = (float)SendMessage(OpacitySlider, TBM_GETPOS, 0, 0);
	_itow(pos, textPos, 10);
	SetWindowTextW(OpacityPos, textPos);
	OutMgr.SetOpacity(pos / 100.f);

	//Restart paddingY to configure second column
	paddingY = addPaddingY + windowRect.top;

	//INITCOMMONCONTROLSEX comctl;
	//comctl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//comctl.dwICC = ICC_STANDARD_CLASSES;
	//InitCommonControlsEx(&comctl);

	float SettingX = width * .33f;
	float SettingY = height * .205f;
	TurnOnOff = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW //| BS_NOTIFY
		, ThreeQuarterWindowX, paddingY, SettingX, SettingY, hWnd, (HMENU)SCV_TURN_ON_OFF, NULL, NULL);
	SetWindowSubclass(TurnOnOff, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	paddingY += addPaddingY + SettingY;


	//HWND Quit = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW
	//	, ThreeQuarterWindowX, paddingY, SettingX, SettingY, hWnd,(HMENU)SCV_QUIT, NULL, NULL);
	//AWT(Quit, SCV_LANG_MGR_QUIT);
	//SetWindowSubclass(Quit, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//paddingY += addPaddingY + SettingY;



//Manual hotkey management
#define OLD_HOTKEY_POS 0
#if OLD_HOTKEY_POS

	float HotkeyValueTextX = width * .30f;
	float HotkeyValueTextY = height * .06f;
	
	float InversePaddingY = height - addPaddingY - HotkeyValueTextY;

	HWND HotkeyValue = CreateWindowEx(0, HOTKEY_CLASS,TEXT(""), WS_CHILD | WS_VISIBLE 
		, ThreeQuarterWindowX, InversePaddingY, HotkeyValueTextX, HotkeyValueTextY, hWnd, (HMENU)SCV_SETTINGS_HOTKEY, NULL, NULL);

	SetWindowSubclass(HotkeyValue, HotkeyProc, 0, 0);

	// Set rules for invalid key combinations. If the user does not supply a
	// modifier key, use ALT as a modifier. If the user supplies SHIFT as a 
	// modifier key, use SHIFT + ALT instead.
	SendMessage(HotkeyValue,
		HKM_SETRULES,
		(WPARAM)HKCOMB_NONE ,   // invalid key combinations 
		MAKELPARAM(HOTKEYF_ALT, 0));       // add ALT to invalid entries 

#if 0
	SendMessage(HotkeyValue,
		HKM_SETRULES,
		(WPARAM)HKCOMB_S,   // invalid key combinations 
		MAKELPARAM(HOTKEYF_SHIFT|HOTKEYF_ALT, 0));
#endif

	InversePaddingY -= HotkeyValueTextY *1.f;
	
	float HotkeyTextY = height * .05f;
	float HotkeyTextX = width * .45f;
	//TODO(fran): will this be a shortcut to turn the veil on and off, or to open smart veil manager??
	//			  or do I make two shortcuts
	std::wstring HotkeyText = L"Shortcut to open Smart Veil Manager";
	HWND Hotkey = CreateWindowW(L"Static", HotkeyText.c_str() , WS_VISIBLE | WS_CHILD
		, ThreeQuarterWindowX, InversePaddingY, HotkeyTextX, HotkeyTextY, hWnd, NULL, NULL, NULL);

	float InversePaddingHotkeyButtonY = InversePaddingY;

	InversePaddingY -= addPaddingY;

	//Register hotkey
	if (CurrentValidHotkeyModifiers || CurrentValidHotkeyVirtualKey) { //if we have some modifiers and a key then try to register
		if (!RegisterHotKey(hWnd, 0, CurrentValidHotkeyModifiers | MOD_NOREPEAT, CurrentValidHotkeyVirtualKey)) {
			SendMessageW(HotkeyValue, SCV_HOTKEY_REG_FAILED, 0, 0);
			//Convert virtual key+modifier into string
			//http://cottonvibes.blogspot.com/2010/11/virtual-key-code-to-string.html
			std::wstring HotkeyStr = HotkeyString(CurrentValidHotkeyModifiers, CurrentValidHotkeyVirtualKey);
			//TODO(fran): this shouldnt be here
			std::wstring hotname = L"Failed to register " + HotkeyStr + L" shortcut" + L", it's already in use by another program";
			MessageBoxW(NULL, hotname.c_str(), L"Smart Veil Error", MB_OK | MB_ICONEXCLAMATION);
		}
		else {
			SendMessageW(HotkeyValue, SCV_HOTKEY_REG_SUCCESS, 0, 0);
		}

		//TODO(fran): if hotkey registration fails and the vk and modifs are not 0 then send to hotkeycontrol the values anyway,
		//			  maybe we could paint the text red to indicate there is a problem and needs changing, but that way we dont delete
		//			  what they already wrote and might forget

		// Set the default hot key for this window. 
		SendMessage(HotkeyValue,
			HKM_SETHOTKEY,
			MAKEWORD(CurrentValidHotkeyVirtualKey, HotkeyModifiersToHotkeyControlModifiers(CurrentValidHotkeyModifiers)),
			0);
	}
#endif

	//Font set-up
	HFONT font;
#if OLD_HOTKEY_POS
	HFONT hotkey_font;
#endif
	LOGFONTW lf;

	memset(&lf, 0, sizeof(lf));
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfHeight = (LONG)(-width * .046f);
	//@ver: lf.pszFaceName y EnumFontFamiliesW
	font = CreateFontIndirectW(&lf);

#if OLD_HOTKEY_POS
	lf.lfHeight /= 2;
	hotkey_font = CreateFontIndirectW(&lf);
#endif

	if (font == NULL)
	{
		MessageBox(hWnd, RCS(SCV_LANG_ERROR_FONT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), MB_OK);
	}
	else {
		SendMessageW(ThresholdText, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(ThresholdPos, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(ThresholdPercentText, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(OpacityText, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(OpacityPos, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(OpacityPercentText, WM_SETFONT, (WPARAM)font, TRUE);
		SendMessageW(TurnOnOff, WM_SETFONT, (WPARAM)font, TRUE);
		//SendMessageW(Quit, WM_SETFONT, (WPARAM)font, TRUE);
	}
#if OLD_HOTKEY_POS
	if (hotkey_font) {
		SendMessageW(Hotkey, WM_SETFONT, (WPARAM)hotkey_font, TRUE);
		SendMessageW(HotkeyValue, WM_SETFONT, (WPARAM)hotkey_font, TRUE);
	}
	else { MessageBox(hWnd, L"Could not create the fonts, will use defaults", L"Smart Veil Error", MB_OK); }
#endif

#if OLD_HOTKEY_POS
	RECT r1, r2;
	GetWindowRect(Hotkey,&r1);
	GetWindowRect(HotkeyValue,&r2);
	float SaveHotBtnX = r2.bottom-r1.top;
	float SaveHotBtnY = SaveHotBtnX;
	SIZE SizeOfHotkeyText;
	HDC textDC = GetDC(Hotkey);

	HFONT hot_font = (HFONT)SendMessage(Hotkey, WM_GETFONT, 0, 0);
	HFONT old_hot_font = nullptr;
	if (hot_font) {//if font == NULL then it is using system font(default I assume)
		old_hot_font = (HFONT)SelectObject(textDC, (HGDIOBJ)hot_font);
	}
	HotkeyText += L"AA";//INFO:to make some space for the button
	GetTextExtentPoint32W(textDC, HotkeyText.c_str(), HotkeyText.length(), &SizeOfHotkeyText);
	HotkeyText.pop_back();//Taking away those two A just in case
	HotkeyText.pop_back();
	if(old_hot_font)SelectObject(textDC, (HGDIOBJ)old_hot_font);//TODO(fran): if old font is NULL I think I should also do SelectObject
	ReleaseDC(Hotkey, textDC);
	//INFO: I need to create the button here so that the dc for the text next to it already has the new font applied
	HWND SaveHotkey = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | BS_ICON //| BS_OWNERDRAW
		, ThreeQuarterWindowX + SizeOfHotkeyText.cx, InversePaddingHotkeyButtonY, SaveHotBtnX, SaveHotBtnY, hWnd, (HMENU)SCV_SAVE_HOTKEY, NULL, NULL);

	SetWindowSubclass(SaveHotkey, ButtonProc, 0, 0);

	HICON save_hotkey = LoadIcon(app_instance, MAKEINTRESOURCE(SAVE_HOTKEY_ICON));
	SendMessage(SaveHotkey, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)save_hotkey);

	std::wstring save_hotkey_tooltip_text = L"Save and apply new hotkey";
	//INFO; both this functions work with buttons
	//CreateToolTipForRect(SaveHotkey, save_hotkey_tooltip_text);
	CreateToolTip(SCV_SAVE_HOTKEY, hWnd, (LPWSTR)save_hotkey_tooltip_text.c_str(), startup_info->show_tooltips);
#endif

	float secretX = width*.1f;
	float secretY = height*.1f;
	HWND secret_button = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW
		, windowRect.left, windowRect.top + height-secretY, secretX, secretY, hWnd, (HMENU)SCV_SECRET_ABOUT, NULL, NULL);

	SetWindowSubclass(secret_button, ControlProcedures::Instance().SecretButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//Settings button control
	//TODO(fran): decide proper placement and size
	POINT settings_pos;
	POINT settings_size; 
	settings_size.x = height * .137f; 
	settings_size.y = settings_size.x;
	//settings_size.x+=2; //INFO: Trying to make the button be less of a square so it is easier for the icon to be correctly centered
	//if ((settings_size.x % 2) != 0) settings_size.x+=1;
#if OLD_HOTKEY_POS
	settings_pos.x = 0;
	settings_pos.y = 0;
#else
	float settings_separation = height*.1f;
	settings_pos.x = windowRect.left + width - settings_size.x - settings_separation;
	settings_pos.y = windowRect.top + height - settings_size.y - settings_separation;
#endif
	HWND Settings = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_ICON //| BS_OWNERDRAW
		, settings_pos.x, settings_pos.y, settings_size.x, settings_size.y, hWnd, (HMENU)SCV_SETTINGS, hInstance, NULL);

	SetWindowSubclass(Settings, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//INFO IMPORTANT: evert button that needs and icon drawn will send the icon value through GWL_USERDATA
	SetWindowLongPtr(Settings, GWL_USERDATA, SETTINGS_ICON);
	
	TOOLTIP_REPO::Instance().CreateToolTip(SCV_SETTINGS, hWnd, SCV_LANG_MGR_SETTINGS);
	//
}

LRESULT CALLBACK WndMgrProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		SetWindowTheme(hWnd, L"", L"");//INFO: to avoid top curved corners on frame
		CREATESTRUCT* creation_info = (CREATESTRUCT*)lParam;
		STARTUP_INFO* startup_info = (STARTUP_INFO*)creation_info->lpCreateParams;
		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE);
		SetupMgr(hWnd,hInstance, startup_info);

		LANGUAGE_MANAGER::Instance().AddDynamicText(hWnd, SCV_LANG_DYNAMIC_UPDATE);

		//Create procedure to add things to the "non client" area
		RECT rc;
		GetWindowRect(hWnd, &rc);

		float button_height = FRAME.caption_height;
		float button_width = button_height * 16.f / 9.f;

		HWND close_button = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | BS_ICON
			, RECTWIDTH(rc) - button_width - FRAME.right_border, 0, button_width, button_height, hWnd, (HMENU)SCV_CUSTOMFRAME_CLOSE, hInstance, NULL);
		SetWindowLongPtr(close_button, GWL_USERDATA, CROSS_ICON);
		SetWindowSubclass(close_button, ControlProcedures::Instance().CaptionButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
		TOOLTIP_REPO::Instance().CreateToolTip(SCV_CUSTOMFRAME_CLOSE, hWnd, SCV_LANG_CLOSE);

		HWND minimize_button = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | BS_ICON
			, RECTWIDTH(rc) - button_width*2 - FRAME.right_border, 0, button_width, button_height, hWnd, (HMENU)SCV_CUSTOMFRAME_MINIMIZE, hInstance, NULL);
		SetWindowLongPtr(minimize_button, GWL_USERDATA, MINIMIZE_ICON);
		SetWindowSubclass(minimize_button, ControlProcedures::Instance().CaptionButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
		TOOLTIP_REPO::Instance().CreateToolTip(SCV_CUSTOMFRAME_MINIMIZE, hWnd, SCV_LANG_MINIMIZE);

		//

		if(startup_info->show_manager_on_startup) ShowWindow(hWnd, SW_SHOW); //TODO(fran): here is why we first see the manager with the old frame, this should be done after first wm_paint
		
		if (startup_info->remember_manager_position) {
			if (startup_info->previous_screen_size.cx == GetSystemMetrics(SM_CXVIRTUALSCREEN)
				&& startup_info->previous_screen_size.cy == GetSystemMetrics(SM_CYVIRTUALSCREEN)) {
				SetWindowPos(hWnd, NULL, startup_info->previous_manager_position.x, startup_info->previous_manager_position.y
					, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);//TODO(fran): check this is rigth, also said that pos was in client coords?
			}
		}
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
		return PaintCaption(hWnd);
	}
	case WM_NCACTIVATE://stuff for custom frame
	{
		return TRUE;
	}
	case WM_NCHITTEST://stuff for custom frame
	{
		return HitTestNCA(hWnd, wParam, lParam);
	}
	case WM_NCCALCSIZE://stuff for custom frame
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
	case WM_NCPAINT://stuff for custom frame
	{
		return 0;
	}
	case SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF:
	{
		//const WCHAR* turnedOnOffText;
		if (isTurnedOn) {
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
	case SCV_LANG_DYNAMIC_UPDATE:
	{
		SendMessage(hWnd, SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF, 0, 0);
		break;
	}
	case WM_HOTKEY:
	{
		UINT keyModifiers = LOWORD(lParam);
		UINT virtualKey = HIWORD(lParam);

		//TODO(fran): right now that we only have one hotkey it is pointless to do this check I think
		if (((keyModifiers^MOD_NOREPEAT) == (CurrentValidHotkeyModifiers^MOD_NOREPEAT)) && (virtualKey == CurrentValidHotkeyVirtualKey)) {
			ShowWindow(hWnd, SW_SHOW);
		}
		break;
	}
	//Manage Slider
	case WM_HSCROLL: //horizontal sliders send WM_HSCROLL, vertical ones send WM_VSCROLL
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
				float pos = (float)SendMessage(slider, TBM_GETPOS, 0, 0);
				WCHAR textPos[4];
				_itow(pos, textPos, 10);
				if (slider == ThresholdSlider) {
					SetWindowTextW(ThresholdPos, textPos);
					OutMgr.SetThreshold(pos/100.f);
				}
				else if (slider == OpacitySlider) {
					SetWindowTextW(OpacityPos, textPos);
					OutMgr.SetOpacity(pos / 100.f);
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
		case SCV_CUSTOMFRAME_CLOSE: 
		{
			//HWND parent = GetParent(hWnd);

			//Save startup info
			STARTUP_INFO startup_info;
			RECT rec;
			GetWindowRect(WindowManagerHandle, &rec);
			POINT mgrpos = { rec.left,rec.top };
			SIZE virtualscreensize = { GetSystemMetrics(SM_CXVIRTUALSCREEN),GetSystemMetrics(SM_CYVIRTUALSCREEN) };//TODO(fran): check this is what we want to be calling on multi-monitor
			int thresholdpos = (int)SendDlgItemMessage(WindowManagerHandle, SCV_TOOLTIP_THRESHOLD_SLIDER, TBM_GETPOS, NULL, NULL);
			int opacitypos = (int)SendDlgItemMessage(WindowManagerHandle, SCV_TOOLTIP_OPACITY_SLIDER, TBM_GETPOS, NULL, NULL);
			FillStartupInfo(startup_info, CurrentValidSettings, CurrentValidHotkeyModifiers, CurrentValidHotkeyVirtualKey, isTurnedOn,
				mgrpos, virtualscreensize, thresholdpos, opacitypos);

			std::wstring info_directory_path = info_path.known_folder + L"\\" + info_path.info_folder;
			std::wstring info_file_name = info_path.info_file + L"." + info_path.info_extension;
			SaveStartupInfo(startup_info, info_directory_path, info_file_name);

			if (WindowHandle != NULL) { DestroyWindow(WindowHandle); }
			PostQuitMessage(0);
			break;
		}
		case SCV_CUSTOMFRAME_MINIMIZE:
		{
			//CloseWindow(hWnd);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		case SCV_SECRET_ABOUT:
		{
			std::wstring copyright = GetVersionInfo(NULL, SCV_VERSION_INFO, L"LegalCopyright");
			std::wstring copyright_year = first_numberstring(copyright);
			std::wstring about_msg = L"Franco Badenas Abal ©";
			if (copyright_year != L"") about_msg += L" " + copyright_year;
			about_msg += L"\n\n";
			about_msg += RS(SCV_LANG_VERSION);
			about_msg += L" " + GetVersionInfo(NULL, SCV_VERSION_INFO, L"ProductVersion");//TODO(fran): if no version found put message saying that or dont show version
			//TODO(fran): make it so only one about window can exist at any single time, without having to put hwnd parameter which locks the manager
			MessageBoxW(NULL, about_msg.c_str(), RCS(SCV_LANG_ABOUT), MB_TOPMOST);
			break;
		}
		case SCV_SETTINGS:
		{
#if 1 //settings in the middle of the manager
			RECT rec; GetWindowRect(hWnd, &rec);
			RECT rec2; GetWindowRect(WindowSettingsHandle, &rec2);
			//TODO(fran):check if settings window is already visible in which case do not change its position
			HMONITOR current_monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO mon_info; mon_info.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(current_monitor, &mon_info);
			//TODO(fran): fix for multi-monitor, also should we fix for left and right?, finally this isnt really necesary, and depending on
			// the multimon setup the user has it's not even necessary
			int desiredYPos = rec.top + RECTHEIGHT(rec)/ 4;
			if (mon_info.rcWork.bottom < (desiredYPos + RECTHEIGHT(rec2))) {
				desiredYPos -= (desiredYPos + RECTHEIGHT(rec2) - mon_info.rcWork.bottom);
			}
			SetWindowPos(WindowSettingsHandle, NULL, rec.left + RECTWIDTH(rec)/2- RECTWIDTH(rec2) /2, desiredYPos
				, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
#else //settings on the right of the manager
			RECT mgr_rc; GetWindowRect(hWnd, &mgr_rc);
			RECT set_rc; GetWindowRect(hWnd, &set_rc);

			POINT set_pos; 
			set_pos.y = mgr_rc.top;
			set_pos.x = mgr_rc.right + max(1,RECTWIDTH(mgr_rc)*.03);

			SetWindowPos(WindowSettingsHandle, NULL, set_pos.x, set_pos.y
				, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
#endif
			ShowWindow(WindowSettingsHandle, SW_SHOW);
			break;
		}
#if OLD_HOTKEY_POS
		case SCV_SAVE_HOTKEY:
		{
			WORD new_hotkey = SendDlgItemMessage(hWnd,SCV_SETTINGS_HOTKEY, HKM_GETHOTKEY, 0, 0);
			BYTE new_modifs = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(new_hotkey));
			BYTE new_vk = LOBYTE(new_hotkey);
			if (new_hotkey) { //cant check if both vk and modif exist cause there are vks that are accepted alone
				if (new_modifs != CurrentValidHotkeyModifiers || new_vk != CurrentValidHotkeyVirtualKey) {
					if (RegisterHotKey(hWnd, 0, new_modifs | MOD_NOREPEAT, new_vk)) {
						//Hotkey registration succeded, we have a new valid hotkey
						SendDlgItemMessage(hWnd, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_REG_SUCCESS, 0, 0);
						CurrentValidHotkeyVirtualKey = new_vk;
						CurrentValidHotkeyModifiers = new_modifs;
						
						//UnregisterHotKey(hWnd, 0);//I think this unregisters only the older hotkey
						//INFO(fran): it seems like we dont need to unregister the previous hotkey, did i misunderstand the docs?

						//TODO(fran): we could disable the button to let the user understand the changes were applied successfully,
						// right now it is not clear wheter it worked or not
					}
					else {
						//Hotkey registration failed
						SendDlgItemMessage(hWnd, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_REG_FAILED, 0, 0);

						//TODO(fran): if failed write the currently valid hotkey instead of red color with bad hotkey ? 
					}
				}
			}
			break;
		}
#endif
		case SCV_TURN_ON_OFF://TODO(fran): this will be separated into updating the bool and another msg for updating the text
		{
			isTurnedOn = !isTurnedOn;
			if (isTurnedOn) {
				//TODO(fran): could it ever happen that we didnt already own the mutex?
				ReleaseMutex(process_next_frame_mutex);
				ShowWindow(WindowHandle, SW_SHOW);
			}
			else {
				ShowWindow(WindowHandle, SW_HIDE);
				OutMgr.RestartTextures();

				//TODO(fran): could it ever happen that we already had the mutex owned?
				DWORD mut_ret = WaitForSingleObject(process_next_frame_mutex, INFINITE);
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
	case SCV_MANAGER_CREATE_TRAY:
	{
		HICON tray_icon;
		LoadIconMetric((HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE), MAKEINTRESOURCE(LOGO_ICON), LIM_SMALL, &tray_icon);
		if (!TRAY_HANDLER::Instance().CreateTrayIcon(WindowManagerHandle, 1, tray_icon, SCV_TRAY, SCV_LANG_TRAY_TIP))
			MessageBox(hWnd, RCS(SCV_LANG_ERROR_TRAY_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
		break;
	}
	case SCV_MANAGER_REMOVE_TRAY:
	{
		TRAY_HANDLER::Instance().DestroyTrayIcon(WindowManagerHandle, 1);
		break;
	}
	case SCV_TRAY: //Tray icon handling
	{
		switch (lParam) {
		case WM_LBUTTONDOWN: 
		{
			SendMessage(hWnd, WM_COMMAND, MAKELONG(SCV_TURN_ON_OFF,0), 0);
			break;
		}
		case WM_RBUTTONDOWN:
		{
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
		ShowWindow(hWnd, SW_HIDE);
		SendMessage(WindowSettingsHandle, WM_CLOSE, 0, 0);
		//Reset the state of the hotkey control in case the user typed something else that wasnt valid
#if OLD_HOTKEY_POS
		SendDlgItemMessage(hWnd,SCV_SETTINGS_HOTKEY,
			HKM_SETHOTKEY,
			MAKEWORD(CurrentValidHotkeyVirtualKey, HotkeyModifiersToHotkeyControlModifiers(CurrentValidHotkeyModifiers)),
			0);
#endif
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
		//INFO(fran): here destroy everything and close the app
		TRAY_HANDLER::Instance().DestroyTrayIcon(WindowManagerHandle, 1);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Hit test the frame for resizing and moving.
LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Get the point coordinates for the hit test.
	POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	// Get the window rectangle.
	RECT rcWindow;
	GetWindowRect(hWnd, &rcWindow);

	// Get the frame rectangle, adjusted for the style without a caption.
	RECT rcFrame = { 0 };
	AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

	// Determine if the hit test is for resizing. Default middle (1,1).
	USHORT uRow = 1;
	USHORT uCol = 1;
	bool fOnResizeBorder = false;

	// Determine if the point is at the top or bottom of the window.
	if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + FRAME.caption_height)
	{
		fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
		uRow = 0;
	}
	else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - FRAME.bottom_border)
	{
		uRow = 2;
	}

	// Determine if the point is at the left or right of the window.
	if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + FRAME.left_border)
	{
		uCol = 0; // left side
	}
	else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - FRAME.right_border)
	{
		uCol = 2; // right side
	}

	// Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
	LRESULT hitTests[3][3] =
	{
		{ HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
		{ HTLEFT,       HTNOWHERE,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	};

	return hitTests[uRow][uCol];
}

LRESULT PaintCaption(HWND hWnd) {
	HDC         hdc;
	PAINTSTRUCT ps;
	RECT        rect;

	hdc = BeginPaint(hWnd, &ps);

	GetWindowRect(hWnd, &rect);
	//INFO: after the first wm_paint call getclientrect and getwindowrect return the same value which is the one returned by getwidowrect
	//GetWindowRect(hwnd, &rect);
	//HGDIOBJ original = NULL;

	//original = SelectObject(hdc, GetStockObject(DC_PEN));
	//SetDCPenColor(hdc, RGB(0, 255, 0));
	//SelectObject(hdc, CreateSolidBrush(RGB(255, 0, 0)));

	//Draw top caption frame
	RECT fill_rc = { FRAME.left_border, 0, RECTWIDTH(rect) - FRAME.right_border, FRAME.caption_height };
	FillRect(hdc, &fill_rc, FRAME.caption_bk_brush);

#if 0 //Assign a "transparent" color to the borders
	HBRUSH transp_br = CreateSolidBrush(RGB(255, 0, 255));
	RECT transp_rc_left = { 0,0,FRAME.left_border ,RECTHEIGHT(rect) };
	FillRect(hdc, &transp_rc_left, transp_br);
	RECT transp_rc_right = { RECTWIDTH(rect) - FRAME.right_border,0,RECTWIDTH(rect),RECTHEIGHT(rect) };
	FillRect(hdc, &transp_rc_right, transp_br);
	RECT transp_rc_bottom = { 0,RECTHEIGHT(rect) - FRAME.bottom_border,RECTWIDTH(rect),RECTHEIGHT(rect) };
	FillRect(hdc, &transp_rc_bottom, transp_br);
	DeleteObject(transp_br);
#endif
	//TODO: try SetWindowRgn to make invisible the borders, documentation says what is outside isnt displayed!

	//SelectObject(hdc, original);
	COLORREF prev_text_col = SetTextColor(hdc, ControlProcedures::Instance().Get_HighlightColor());
	LOGBRUSH bk_brush_info;
	GetObject(FRAME.caption_bk_brush, sizeof(bk_brush_info), &bk_brush_info);
	COLORREF prev_bk_col = SetBkColor(hdc, bk_brush_info.lbColor);

	//Draw logo icon
	SIZE logo_size;
	//TODO(fran): better to use getsystemmetric?
	logo_size.cx = FRAME.caption_height*.6f;
	logo_size.cy = logo_size.cx;
	HICON logo_icon = (HICON)LoadImage((HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE), MAKEINTRESOURCE(LOGO_ICON), IMAGE_ICON
		, logo_size.cx, logo_size.cy, LR_SHARED);

	int logo_x_offset = GetSystemMetrics(SM_CXSMICON);
	if (logo_icon) {
		DrawIconEx(hdc, logo_x_offset, (FRAME.caption_height - logo_size.cy) / 2,
			logo_icon, logo_size.cx, logo_size.cy, 0, NULL, DI_NORMAL);

		DestroyIcon(logo_icon);
	}

	HFONT font;
	LOGFONTW lf;

	memset(&lf, 0, sizeof(lf));
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfHeight = (LONG)(-RECTHEIGHT(fill_rc) * .5);
	//@ver: lf.pszFaceName y EnumFontFamiliesW
	font = CreateFontIndirectW(&lf);

	HFONT prev_font = (HFONT)SelectObject(hdc, (HGDIOBJ)font);

	WCHAR title[200];
	SendMessage(hWnd, WM_GETTEXT, 200, (LPARAM)title);
	RECT text_rc = { logo_x_offset * 2 + logo_size.cx,0,min(200,RECTWIDTH(rect) - FRAME.right_border - FRAME.left_border),FRAME.caption_height };
	DrawText(hdc, title, -1, &text_rc, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	SetTextColor(hdc, prev_text_col);
	SetBkColor(hdc, prev_bk_col);
	SelectObject(hdc, (HGDIOBJ)prev_font);
	EndPaint(hWnd, &ps);

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

//
// Entry point for new duplication threads
//
DWORD WINAPI DDProc(_In_ void* Param)
{
    // Classes
    DISPLAYMANAGER DispMgr;
    DUPLICATIONMANAGER DuplMgr;

    // D3D objects
    ID3D11Texture2D* SharedSurf = nullptr;
    IDXGIKeyedMutex* KeyMutex = nullptr;

    // Data passed in from thread creation
    THREAD_DATA* TData = reinterpret_cast<THREAD_DATA*>(Param);

    // Get desktop
    DUPL_RETURN Ret;
    HDESK CurrentDesktop = nullptr;
    CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
    if (!CurrentDesktop)
    {
        // We do not have access to the desktop so request a retry
        SetEvent(TData->ExpectedErrorEvent);
        Ret = DUPL_RETURN_ERROR_EXPECTED;
        goto Exit;
    }

    // Attach desktop to this thread
    bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
    CloseDesktop(CurrentDesktop);
    CurrentDesktop = nullptr;
    if (!DesktopAttached)
    {
        // We do not have access to the desktop so request a retry
        Ret = DUPL_RETURN_ERROR_EXPECTED;
        goto Exit;
    }

    // New display manager
    DispMgr.InitD3D(&TData->DxRes);

    // Obtain handle to sync shared Surface
    HRESULT hr = TData->DxRes.Device->OpenSharedResource(TData->TexSharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SharedSurf));
    if (FAILED (hr))
    {
        Ret = ProcessFailure(TData->DxRes.Device, RCS(SCV_LANG_ERROR_SHARED_SURF_OPEN), RCS(SCV_LANG_ERROR_SMARTVEIL), hr, SystemTransitionsExpectedErrors);
        goto Exit;
    }

    hr = SharedSurf->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&KeyMutex));
    if (FAILED(hr))
    {
        Ret = ProcessFailure(nullptr, RCS(SCV_LANG_ERROR_KEYEDMUTEX_GET), RCS(SCV_LANG_ERROR_SMARTVEIL), hr);
        goto Exit;
    }

    // Make duplication manager
    Ret = DuplMgr.InitDupl(TData->DxRes.Device, TData->Output);
    if (Ret != DUPL_RETURN_SUCCESS)
    {
        goto Exit;
    }

    // Get output description
    DXGI_OUTPUT_DESC DesktopDesc;
    RtlZeroMemory(&DesktopDesc, sizeof(DXGI_OUTPUT_DESC));
    DuplMgr.GetOutputDesc(&DesktopDesc);

    // Main duplication loop
    bool WaitToProcessCurrentFrame = false;
    FRAME_DATA CurrentData;

    while ((WaitForSingleObjectEx(TData->TerminateThreadsEvent, 0, FALSE) == WAIT_TIMEOUT))
    {
        if (!WaitToProcessCurrentFrame)
        {
            // Get new frame from desktop duplication
            bool TimeOut;
            Ret = DuplMgr.GetFrame(&CurrentData, &TimeOut);
            if (Ret != DUPL_RETURN_SUCCESS)
            {
                // An error occurred getting the next frame drop out of loop which
                // will check if it was expected or not
                break;
            }

            // Check for timeout
            if (TimeOut)
            {
                // No new frame at the moment
                continue;
            }
        }

        // We have a new frame so try and process it
        // Try to acquire keyed mutex in order to access shared surface
        hr = KeyMutex->AcquireSync(0, 1000);
        if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
        {
            // Can't use shared surface right now, try again later
            WaitToProcessCurrentFrame = true;
            continue;
        }
        else if (FAILED(hr))
        {
            // Generic unknown failure
            Ret = ProcessFailure(TData->DxRes.Device, RCS(SCV_LANG_ERROR_KEYEDMUTEX_ACQUIRE_UNEXPECTED), RCS(SCV_LANG_ERROR_SMARTVEIL), hr, SystemTransitionsExpectedErrors);
            DuplMgr.DoneWithFrame();
            break;
        }

        // We can now process the current frame
        WaitToProcessCurrentFrame = false;

        // Get mouse info
#if 0
        Ret = DuplMgr.GetMouse(TData->PtrInfo, &(CurrentData.FrameInfo), TData->OffsetX, TData->OffsetY);
        if (Ret != DUPL_RETURN_SUCCESS)
        {
            DuplMgr.DoneWithFrame();
            KeyMutex->ReleaseSync(1);
            break;
        }
#endif

        // Process new frame
        Ret = DispMgr.ProcessFrame(&CurrentData, SharedSurf, TData->OffsetX, TData->OffsetY, &DesktopDesc);
        if (Ret != DUPL_RETURN_SUCCESS)
        {
            DuplMgr.DoneWithFrame();
            KeyMutex->ReleaseSync(1);
            break;
        }

        // Release acquired keyed mutex
        hr = KeyMutex->ReleaseSync(1);
        if (FAILED(hr))
        {
            Ret = ProcessFailure(TData->DxRes.Device, RCS(SCV_LANG_ERROR_KEYEDMUTEX_RELEASE_UNEXPECTED), RCS(SCV_LANG_ERROR_SMARTVEIL), hr, SystemTransitionsExpectedErrors);
            DuplMgr.DoneWithFrame();
            break;
        }

        // Release frame back to desktop duplication
        Ret = DuplMgr.DoneWithFrame();
        if (Ret != DUPL_RETURN_SUCCESS)
        {
            break;
        }
    }

Exit:
    if (Ret != DUPL_RETURN_SUCCESS)
    {
        if (Ret == DUPL_RETURN_ERROR_EXPECTED)
        {
            // The system is in a transition state so request the duplication be restarted
            SetEvent(TData->ExpectedErrorEvent);
        }
        else
        {
            // Unexpected error so exit the application
            SetEvent(TData->UnexpectedErrorEvent);
        }
    }

    if (SharedSurf)
    {
        SharedSurf->Release();
        SharedSurf = nullptr;
    }

    if (KeyMutex)
    {
        KeyMutex->Release();
        KeyMutex = nullptr;
    }

    return 0;
}

_Post_satisfies_(return != DUPL_RETURN_SUCCESS)
DUPL_RETURN ProcessFailure(_In_opt_ ID3D11Device* Device, _In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr, _In_opt_z_ HRESULT* ExpectedErrors)
{
    HRESULT TranslatedHr;

    // On an error check if the DX device is lost
    if (Device)
    {
        HRESULT DeviceRemovedReason = Device->GetDeviceRemovedReason();

        switch (DeviceRemovedReason)
        {
            case DXGI_ERROR_DEVICE_REMOVED :
            case DXGI_ERROR_DEVICE_RESET :
            case static_cast<HRESULT>(E_OUTOFMEMORY) :
            {
                // Our device has been stopped due to an external event on the GPU so map them all to
                // device removed and continue processing the condition
                TranslatedHr = DXGI_ERROR_DEVICE_REMOVED;
                break;
            }

            case S_OK :
            {
                // Device is not removed so use original error
                TranslatedHr = hr;
                break;
            }

            default :
            {
                // Device is removed but not a error we want to remap
                TranslatedHr = DeviceRemovedReason;
            }
        }
    }
    else
    {
        TranslatedHr = hr;
    }

    // Check if this error was expected or not
    if (ExpectedErrors)
    {
        HRESULT* CurrentResult = ExpectedErrors;

        while (*CurrentResult != S_OK)
        {
            if (*(CurrentResult++) == TranslatedHr)
            {
                return DUPL_RETURN_ERROR_EXPECTED;
            }
        }
    }

    // Error was not expected so display the message box
    DisplayMsg(Str, Title, TranslatedHr);

    return DUPL_RETURN_ERROR_UNEXPECTED;
}

//
// Displays a message
//
void DisplayMsg(_In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr)
{
    if (SUCCEEDED(hr))
    {
        MessageBoxW(nullptr, Str, Title, MB_OK);
        return;
    }

    const UINT StringLen = (UINT)(wcslen(Str) + sizeof(" HRESULT 0x########"))+1;
    wchar_t* OutStr = new wchar_t[StringLen];
    if (!OutStr)
    {
        return;
    }

    INT LenWritten = swprintf_s(OutStr, StringLen, L"%s HRESULT 0x%X", Str, hr);
    if (LenWritten != -1)
    {
        MessageBoxW(nullptr, OutStr, Title, MB_OK);
    }

    delete [] OutStr;
}
