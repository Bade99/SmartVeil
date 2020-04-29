#include "Settings.h"

#include "resource.h"

#include <uxtheme.h>// setwindowtheme
#include "Common.h"
#include "TRAY_HANDLER.h"
#include "utils.cpp"
#include "ControlProcedures.h"
#include "TOOLTIP_REPO.h"

#include "Manager.h" //TODO(fran): I need this just for one #define, lets move all those to a common file

//Definition of child control ids and internal messages
#define SCV_SETTINGS_LANG_COMBO (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+1) //The combobox that contains the language selection
#define SCV_SETTINGS_DANGEROUS_SLIDER (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+2) //The checkbox that contains the "reduce dangerous sliders" option
#define SCV_SETTINGS_MANAGER_POS (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+3) //The checkbox that contains the "remember mgr pos" option
#define SCV_SETTINGS_MANAGER_ON_STARTUP (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+4) //The checkbox that contains the "show mgr on application startup" option
#define SCV_SETTINGS_SHOW_TOOLTIPS (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+5) //The checkbox that contains the "show tooltips" option
#define SCV_SETTINGS_SHOW_TRAY (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+6) //The checkbox that contains the "show tray icon" option
#define SCV_SETTINGS_VEIL_STARTUP_COMBO (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+7) //The combobox that contains the "turn on veil on startup" selection
#define SCV_SETTINGS_START_WITH_WINDOWS (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+8) //The checkbox that contains the "start with windows" option
#define SCV_SETTINGS_SAVE (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+9) //Save button
#define SCV_SETTINGS_HOTKEY (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+10) //Hotkey control
#define SCV_SETTINGS_UPDATE_COUNTER_TEXT (SCV_SETTINGS_FIRST_INTERNAL_MESSAGE+11) //Static text control that shows the veil update rate - DEBUG ONLY


/// <summary>
/// Full control setup for the Settings window
/// </summary>
/// <param name="hwnd"></param>
/// <param name="hInstance"></param>
/// <param name="current_settings"></param>
void SetupSettings(HWND hwnd, HINSTANCE hInstance,const CUSTOM_FRAME& frame, const SETTINGS& settings, HFONT &settings_font) {
	//Settings needs:
	//損anguage (combobox)
	//斟educe dangerous slider values (checkbox)
	//斟emember manager pos (checkbox)
	//新how manager on app startup (checkbox)
	//新how tooltips (checkbox)
	//新how tray icon (checkbox)
	//新how veil on startup (combobox)
	//新tart with windows (checkbox)
	//搬otkey (hotkey), hotkey at the end since it's the most probable that will change, it will be closer to the save button
	//新ave (button), if successful save then close settings, if error eg bad hotkey then leave it open and tell the problem

	//TODO(fran): checkbox to let the user decide what do left & right click on tray icon do?

	//TODO(fran): we need a hidden control with the font already set so we can test the size of text and adjust automatically


	RECT rec;
	GetMyClientRect(hwnd,frame, &rec);

	float width = (float)RECTWIDTH(rec);
	float height = (float)RECTHEIGHT(rec);

	float paddingX = rec.left + width * .05f;
	float paddingY = height * .01f;
	float addPaddingY = paddingY;
	paddingY += rec.top;

	float TextY = height * .08f;//default text height

	POINT checkbox_text;
	checkbox_text.x = (LONG)(width * .9f);
	checkbox_text.y = (LONG)TextY;
	HWND manager_on_startup = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_MANAGER_ON_STARTUP, hInstance, NULL);
	AWT(manager_on_startup, SCV_LANG_SETTINGS_SHOW_MGR);
	//SetWindowLongPtr(manager_on_startup, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(manager_on_startup, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(manager_on_startup, BM_SETCHECK, settings.show_manager_on_startup ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	HWND show_tray = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_SHOW_TRAY, hInstance, NULL);
	AWT(show_tray, SCV_LANG_SETTINGS_SHOW_TRAY);
	//SetWindowLongPtr(show_tray, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(show_tray, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(show_tray, BM_SETCHECK, settings.show_tray_icon ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	//TODO(fran): move this guy below, after we already said the on startup == on application startup
	HWND dangerous_slider = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_DANGEROUS_SLIDER, hInstance, NULL);
	AWT(dangerous_slider, SCV_LANG_SETTINGS_REDUCE_SLIDER);
	//SetWindowLongPtr(dangerous_slider, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(dangerous_slider, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(dangerous_slider, BM_SETCHECK, settings.reduce_dangerous_slider_values ? BST_CHECKED : BST_UNCHECKED, 0);

	//INFO: See displaying text in https://docs.microsoft.com/en-us/windows/win32/controls/tooltip-controls
	TOOLTIP_REPO::Instance().CreateToolTip(dangerous_slider, SCV_LANG_SETTINGS_REDUCE_SLIDER_TIP);

	paddingY += checkbox_text.y + addPaddingY;

	HWND remember_manager_pos = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_MANAGER_POS, hInstance, NULL);
	AWT(remember_manager_pos, SCV_LANG_SETTINGS_REMEMBER_MGR_POS);
	//SetWindowLongPtr(remember_manager_pos, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(remember_manager_pos, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(remember_manager_pos, BM_SETCHECK, settings.remember_manager_position ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	HWND show_tooltips = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_SHOW_TOOLTIPS, hInstance, NULL);
	AWT(show_tooltips, SCV_LANG_SETTINGS_SHOW_TOOLTIP);
	//SetWindowLongPtr(show_tooltips, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(show_tooltips, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(show_tooltips, BM_SETCHECK, settings.show_tooltips ? BST_CHECKED : BST_UNCHECKED, 0);

	TOOLTIP_REPO::Instance().CreateToolTip(show_tooltips, SCV_LANG_SETTINGS_SHOW_TOOLTIP_TIP);

	paddingY += checkbox_text.y + addPaddingY;

	HWND start_with_windows = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, (HMENU)SCV_SETTINGS_START_WITH_WINDOWS, hInstance, NULL);
	AWT(start_with_windows, SCV_LANG_SETTINGS_START_WITH_WIN);
	//SetWindowLongPtr(start_with_windows, GWL_USERDATA, CHECKBOX_ICON);
	SetWindowSubclass(start_with_windows, ControlProcedures::Instance().CheckboxProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	SendMessageW(start_with_windows, BM_SETCHECK, settings.start_with_windows ? BST_CHECKED : BST_UNCHECKED, 0);

	paddingY += checkbox_text.y + addPaddingY;

	POINT veil_on_startup_text;
	veil_on_startup_text.x = (LONG)(width * .50f);
	veil_on_startup_text.y = (LONG)TextY;
	//TODO(fran): proper static control text alignment
	HWND veil_on_startup = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE
		, (int)paddingX, (int)paddingY, veil_on_startup_text.x, veil_on_startup_text.y, hwnd, NULL, NULL, NULL);
	AWT(veil_on_startup, SCV_LANG_SETTINGS_TURN_ON);

	POINT combobox_text;
	combobox_text.x = (LONG)(width * .315f);
	combobox_text.y = veil_on_startup_text.y;

	HWND veil_on_startup_combo = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP
		, (int)(paddingX + veil_on_startup_text.x*1.05f), (int)paddingY, combobox_text.x, combobox_text.y, hwnd, (HMENU)SCV_SETTINGS_VEIL_STARTUP_COMBO, hInstance, NULL);

	ACT(veil_on_startup_combo, VEIL_ON_STARTUP::YES, SCV_LANG_SETTINGS_TURN_ON_YES);//INFO(fran):this values MUST be lined up with LANGUAGE enum
	ACT(veil_on_startup_combo, VEIL_ON_STARTUP::NO, SCV_LANG_SETTINGS_TURN_ON_NO);
	ACT(veil_on_startup_combo, VEIL_ON_STARTUP::REMEMBER_LAST_STATE, SCV_LANG_SETTINGS_TURN_ON_REMEMBER);
	SendMessageW(veil_on_startup_combo, CB_SETCURSEL, settings.show_veil_on_startup, 0);
	//SetWindowLongPtr(veil_on_startup_combo, GWL_USERDATA, COMBO_ICON);
	SetWindowSubclass(veil_on_startup_combo, ControlProcedures::Instance().ComboProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	paddingY += combobox_text.y + addPaddingY;

	POINT language_text;
	language_text.x = (LONG)(width * .25f);
	language_text.y = (LONG)TextY;
	HWND LanguageText = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE
		, (int)paddingX, (int)paddingY, language_text.x, language_text.y, hwnd, NULL, NULL, NULL);
	AWT(LanguageText, SCV_LANG_SETTINGS_LANG);

	HWND language_combo = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP
		, (int)(paddingX + language_text.x*1.1f), (int)paddingY, combobox_text.x, combobox_text.y, hwnd, (HMENU)SCV_SETTINGS_LANG_COMBO, hInstance, NULL);

	SendMessageW(language_combo, CB_ADDSTRING, (WPARAM)LANGUAGE_MANAGER::LANGUAGE::ENGLISH, (LPARAM)L"English");//INFO(fran):this values MUST be lined up with LANGUAGE enum
	SendMessageW(language_combo, CB_ADDSTRING, (WPARAM)LANGUAGE_MANAGER::LANGUAGE::SPANISH, (LPARAM)L"Espa隳l");//INFO: this strings will not change with langs, each lang will be written like it should
	SendMessageW(language_combo, CB_SETCURSEL, (WPARAM)settings.language, 0);
	//SetWindowLongPtr(language_combo, GWL_USERDATA, COMBO_ICON);
	SetWindowSubclass(language_combo, ControlProcedures::Instance().ComboProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	paddingY += combobox_text.y + addPaddingY;

	//TODO(fran): will this be a shortcut to turn the veil on and off, or to open smart veil manager??
	//			  or do I make two shortcuts
	HWND Hotkey_text = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE
		, (int)paddingX, (int)paddingY, checkbox_text.x, checkbox_text.y, hwnd, NULL, NULL, NULL);
	AWT(Hotkey_text, SCV_LANG_SETTINGS_SHORTCUT);
	paddingY += checkbox_text.y + addPaddingY;

	// Ensure that the common control DLL is loaded. 
	INITCOMMONCONTROLSEX icex; // declare an INITCOMMONCONTROLSEX Structure
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_HOTKEY_CLASS; // set dwICC member to ICC_HOTKEY_CLASS    
	InitCommonControlsEx(&icex); // this loads the Hot Key control class.

	POINT hotkey_value_text;
	hotkey_value_text.x = (LONG)(width * .6f);
	hotkey_value_text.y = (LONG)(TextY);
	HWND HotkeyValue = CreateWindowEx(0, HOTKEY_CLASS, TEXT(""), WS_CHILD | WS_VISIBLE
		, (int)paddingX, (int)paddingY, hotkey_value_text.x, hotkey_value_text.y, hwnd, (HMENU)SCV_SETTINGS_HOTKEY, NULL, NULL);
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
	//TODO(fran): this shouldnt be here----------------------------------------------------------------------------------------------------------
	if (settings.hotkey.vk) { //if we have a key and maybe some modifiers then try to register
		if (!RegisterHotKey(GetWindow(hwnd, GW_OWNER), 0,settings.hotkey.mods | MOD_NOREPEAT, settings.hotkey.vk)) {
			SendMessageW(HotkeyValue, SCV_HOTKEY_REG_FAILED, 0, 0);
			//Convert virtual key+modifier into string
			//http://cottonvibes.blogspot.com/2010/11/virtual-key-code-to-string.html
			std::wstring HotkeyStr = HotkeyString((WORD)settings.hotkey.mods, (BYTE)settings.hotkey.vk); //TODO(fran): do this conversions mean I originally created the struct with the wrong types?
			std::wstring hotname = RS(SCV_LANG_SETTINGS_SHORTCUT_REG_FAIL_1);
			hotname += L" " + HotkeyStr + L"\n" + RS(SCV_LANG_SETTINGS_SHORTCUT_REG_FAIL_2);
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
			MAKEWORD(((WORD)settings.hotkey.vk), HotkeyModifiersToHotkeyControlModifiers(settings.hotkey.mods)),
			0);
	}

	//TODO(fran): do we make this the floppy icon?
	POINT save_text;
	save_text.x = (LONG)(width * .25f);
	save_text.y = (LONG)TextY;
	HWND Save = CreateWindowW(L"Button", NULL, WS_VISIBLE | WS_CHILD //| BS_OWNERDRAW //| BS_NOTIFY
		, (int)(rec.left + width - save_text.x - paddingX), (int)paddingY, save_text.x, save_text.y, hwnd, (HMENU)SCV_SETTINGS_SAVE, NULL, NULL);
	AWT(Save, SCV_LANG_SETTINGS_SAVE);

	paddingY += save_text.y + addPaddingY;

	SetWindowSubclass(Save, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());

	//Basic update counter
	POINT UpdateCounterSize = { MS_UPDATE ? (LONG)(width * .2f) : (LONG)(width * .1f) ,(LONG)(height * .08f) };
	CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD
		, rec.left, (int)(rec.top + height - UpdateCounterSize.y), UpdateCounterSize.x, UpdateCounterSize.y, hwnd, (HMENU)SCV_SETTINGS_UPDATE_COUNTER_TEXT, NULL, NULL);

#ifdef _DEBUG
	POINT UpdateCounterUnitSize = { (LONG)(width *.08f),(LONG)(height*.1f) };
	CreateWindowW(L"Static", MS_UPDATE ? L"ms" : L"fps", WS_VISIBLE | WS_CHILD
		, rec.left + UpdateCounterSize.x, (int)(rec.top + height - UpdateCounterSize.y), UpdateCounterUnitSize.x, UpdateCounterUnitSize.y, hwnd, NULL, NULL, NULL);
#endif

	//Font set-up
	settings_font = CreateMyFont((LONG)(-width * .046f));

	if (settings_font == NULL)
	{
		MessageBox(hwnd, RCS(SCV_LANG_ERROR_FONT_CREATE), RCS(SCV_LANG_ERROR_SMARTVEIL), MB_OK);
	}
	else {
		EnumChildWindows(hwnd, [](HWND child, LPARAM font) ->BOOL {SendMessageW(child, WM_SETFONT, (WPARAM)(HFONT)font, TRUE); return TRUE; }
		, (LPARAM)settings_font);
	}
}

/// <summary>
/// Apply the new hotkey to the mgr and tell the settings' hotkey control if we succeeded or not
/// </summary>
/// <param name="mgr">The window that will receive messages when a hotkey is pressed</param>
/// <param name="settings">The window that has the hotkey control and therefore the needed info</param>
/// <param name="_settings">The object where the new settings will be saved</param>
/// <returns>TRUE if the new hotkey was applied, FALSE if failed, beware the previous hotkey is removed, if you want it back apply again with it</returns>
BOOL Settings_ApplyHotkey(HWND mgr, HWND settings,const SETTINGS* _settings) {
	//WORD new_hotkey = SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, HKM_GETHOTKEY, 0, 0);
	//BYTE new_modifs = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(new_hotkey));
	//BYTE new_vk = LOBYTE(new_hotkey);



	//if (new_hotkey) { //cant check if both vk and modif exist cause there are vks that are accepted alone
		//if (new_modifs != _settings->hotkey.mods || new_vk != _settings->hotkey.vk) {

			//+THIS IS NEW
			//if (_settings->hotkey.vk || _settings->hotkey.mods) {
				UnregisterHotKey(mgr, 0); //INFO: unregister previous hotkey in case there was any, this has to be done only cause of the stupid hotkey control, the new
											// hotkey replaces the old one but the control doesnt update on that and does not let you input the old one
			//}
			//+

			if (RegisterHotKey(mgr, 0, _settings->hotkey.mods | MOD_NOREPEAT, _settings->hotkey.vk)) {
				//Hotkey registration succeded, we have a new valid hotkey
				SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_REG_SUCCESS, 0, 0);
				//_settings->hotkey.vk = new_vk;
				//_settings->hotkey.mods = new_modifs;

				//TODO(fran): we could disable the button to let the user understand the changes were applied successfully,
				// right now it is not clear wheter it worked or not
				return TRUE;
			}
			else {

				//+THIS IS NEW
				//Re-register the previous hotkey
				//if (_settings->hotkey.vk || _settings->hotkey.mods) { //TODO(fran): check this doesnt introduce any bugs
				//	if (!RegisterHotKey(mgr, 0, _settings->hotkey.mods | MOD_NOREPEAT, _settings->hotkey.vk)) {
				//		_settings->hotkey.vk = NULL;
				//		_settings->hotkey.mods = NULL;
				//		//Clear text for hotkey control
				//		SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, HKM_SETHOTKEY, 0, 0);
				//		//Error msg to the user telling both the new and previous hotkey failed, and we need a new one
				//		MessageBox(settings, RCS(SCV_LANG_ERROR_OLD_AND_NEW_HOTKEYS_REG), RCS(SCV_LANG_ERROR_SMARTVEIL), NULL);
				//	}
				//}
				//+

				//Hotkey registration failed
				SendDlgItemMessage(settings, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_REG_FAILED, 0, 0);
				return FALSE;

				//TODO(fran): if failed write the currently valid hotkey instead of red color with bad hotkey ? 
			}
		//}
	//}
}

/// <summary>
/// Registers program to start with the user session
/// </summary>
/// <returns>ERROR_SUCCESS if succeeded, otherwise error code, use FormatMessage with FORMAT_MESSAGE_FROM_SYSTEM to get error description</returns>
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
		HKEY OnStartup = NULL;
		//DWORD disposition;//INFO: can be REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY

		//first you create the registry if it doesnt already exist
		res = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", NULL, NULL,
			REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE | STANDARD_RIGHTS_REQUIRED, NULL, &OnStartup, NULL);
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

/// <returns>ERROR_SUCCESS if succeeded, otherwise error code, use FormatMessage with FORMAT_MESSAGE_FROM_SYSTEM to get error description</returns>
LSTATUS UnregisterProgramFromStartup() {
	LSTATUS res = RegDeleteKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"Smart Veil");
	return res;
}

/// <summary>
/// Updates the Current Settings with the new info from the Settings window
/// </summary>
/// <param name="settings_window"></param>
/// <param name="current_settings"></param>
inline void SaveCurrentValidSettings(HWND settings_window, SETTINGS* current_settings) {//TODO(fran): should this and the restore procedure be in startup_and_settings.cpp ? hard to decouple
	//We ask each control for its current state and save it to current_settings

	//current_settings.hotkey_modifiers = CurrentValidHotkeyModifiers; //we ask currentvalidhotkey since it may or may not be modified depending on the validity of the hotkey
	//current_settings.hotkey_virtual_key = CurrentValidHotkeyVirtualKey;

	current_settings->language = (LANGUAGE_MANAGER::LANGUAGE)SendDlgItemMessage(settings_window, SCV_SETTINGS_LANG_COMBO, CB_GETCURSEL, 0, 0);
	current_settings->show_veil_on_startup = (VEIL_ON_STARTUP)SendDlgItemMessage(settings_window, SCV_SETTINGS_VEIL_STARTUP_COMBO, CB_GETCURSEL, 0, 0);

	current_settings->reduce_dangerous_slider_values = isChecked(GetDlgItem(settings_window, SCV_SETTINGS_DANGEROUS_SLIDER));
	current_settings->remember_manager_position = isChecked(GetDlgItem(settings_window, SCV_SETTINGS_MANAGER_POS));
	current_settings->show_manager_on_startup = isChecked(GetDlgItem(settings_window, SCV_SETTINGS_MANAGER_ON_STARTUP));
	current_settings->show_tooltips = isChecked(GetDlgItem(settings_window, SCV_SETTINGS_SHOW_TOOLTIPS));
	current_settings->show_tray_icon = isChecked(GetDlgItem(settings_window, SCV_SETTINGS_SHOW_TRAY));
	current_settings->start_with_windows = isChecked(GetDlgItem(settings_window, SCV_SETTINGS_START_WITH_WINDOWS));

	WORD new_hotkey = (WORD)SendDlgItemMessage(settings_window, SCV_SETTINGS_HOTKEY, HKM_GETHOTKEY, 0, 0);
	WORD new_modifs = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(new_hotkey));
	BYTE new_vk = LOBYTE(new_hotkey);

	current_settings->hotkey.mods = new_modifs;
	current_settings->hotkey.vk = new_vk;
}

/// <summary>
/// Used to restore the controls of the Settings window in case the user changes something but does not save
/// </summary>
/// <param name="settings_window"></param>
/// <param name="settings">The last valid settings</param>
void RestoreCurrentValidSettingsToControls(HWND settings_window, const SETTINGS& settings) {

	if (settings.hotkey.vk) {
		//If what is written in the hotkey is different from what currentvalidmod/vk has then reset to the valid ones (if not null)
		WORD hotkey_on_control = (WORD)SendDlgItemMessage(settings_window, SCV_SETTINGS_HOTKEY, HKM_GETHOTKEY, 0, 0);
		WORD modifs_on_control = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(hotkey_on_control));
		BYTE vk_on_control = LOBYTE(hotkey_on_control);
		if (settings.hotkey.mods != modifs_on_control || settings.hotkey.vk != vk_on_control)
			SendDlgItemMessage(settings_window, SCV_SETTINGS_HOTKEY, SCV_HOTKEY_RESET_TEXT, (WPARAM)settings.hotkey.vk, (LPARAM)settings.hotkey.mods);
	}

	SendDlgItemMessage(settings_window, SCV_SETTINGS_LANG_COMBO, CB_SETCURSEL, (WPARAM)settings.language, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_VEIL_STARTUP_COMBO, CB_SETCURSEL, settings.show_veil_on_startup, 0);

	SendDlgItemMessage(settings_window, SCV_SETTINGS_DANGEROUS_SLIDER, BM_SETCHECK, settings.reduce_dangerous_slider_values ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_MANAGER_POS, BM_SETCHECK, settings.remember_manager_position ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_MANAGER_ON_STARTUP, BM_SETCHECK, settings.show_manager_on_startup ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_SHOW_TOOLTIPS, BM_SETCHECK, settings.show_tooltips ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_SHOW_TRAY, BM_SETCHECK, settings.show_tray_icon ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(settings_window, SCV_SETTINGS_START_WITH_WINDOWS, BM_SETCHECK, settings.start_with_windows ? BST_CHECKED : BST_UNCHECKED, 0);
}

LRESULT CALLBACK SettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int numberOfUpdates = 0;
	/// <summary>
	/// Font that will be used by Settings Window's controls
	/// </summary>
	static HFONT settings_font;
	/// <summary>
	/// Current settings, updated realtime
	/// </summary>
	static SETTINGS* CurrentValidSettings;
	switch (message) {
	case WM_CREATE:
	{
		//Creating and setting up controls
		SetWindowTheme(hWnd, L"", L"");//INFO: to avoid top curved corners on frame
		CREATESTRUCT* creation_info = (CREATESTRUCT*)lParam;
		CurrentValidSettings = (SETTINGS*)creation_info->lpCreateParams;
		HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWL_HINSTANCE);

		//TODO(fran): initialize hotkey modif and vk in MAIN

		LANGUAGE_MANAGER::Instance().SetHInstance(hInstance);
		LANGUAGE_MANAGER::Instance().ChangeLanguage(CurrentValidSettings->language);
		TOOLTIP_REPO::Instance().SetHInstance(hInstance);
		TOOLTIP_REPO::Instance().ActivateTooltips(CurrentValidSettings->show_tooltips);

		if (CurrentValidSettings->start_with_windows) {
			//Always check that the value stored in the registry is the same as the value of this exe
			RegisterProgramOnStartup(GetExePath());
		}

		//Initialize Tray Icon	
		//https://docs.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-notifyicondataa
		//see https://social.msdn.microsoft.com/Forums/windows/en-US/a4d7e039-6654-4068-80b2-cd380530d92e/examples-using-win32-api-and-c-for-notification-tray-program?forum=vcgeneral
		//good example
		if (CurrentValidSettings->show_tray_icon) {
			PostMessage(GetWindow(hWnd, GW_OWNER), SCV_MANAGER_MODIFY_TRAY, TRUE, 0);
		}

		SetupSettings(hWnd, hInstance, FRAME, *CurrentValidSettings, settings_font); //TODO(fran): maybe it's better if the font is already passed to SetupSettings
																						//instead of being created there

		//Create procedure to add things to the "non client" area, in this case close button
		RECT rc;
		GetWindowRect(hWnd, &rc);

		float button_height = (float)FRAME.caption_height;
		float button_width = button_height * 16.f / 9.f;

		HWND close_button = CreateWindowW(L"Button", L"", WS_VISIBLE | WS_CHILD | WS_MAXIMIZEBOX
			, (int)(RECTWIDTH(rc) - button_width - FRAME.right_border), 0, (int)button_width, (int)button_height, hWnd, (HMENU)SCV_CUSTOMFRAME_CLOSE, hInstance, NULL);
		//SetWindowLongPtr(close_button, GWL_USERDATA, CROSS_ICON);
		SetWindowSubclass(close_button, ControlProcedures::Instance().CaptionButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
		TOOLTIP_REPO::Instance().CreateToolTip(close_button, SCV_LANG_CLOSE);
		//

#ifdef _DEBUG
		//Starts timer to check how many times does the Veil update per second
		SetTimer(hWnd, 1, 1000, NULL);
#endif

		break;
	}
	case SCV_SETTINGS_GET_SETTINGS:
	{
		*(SETTINGS*)wParam = *CurrentValidSettings;
		return TRUE;
	}
	case WM_PAINT:
	{
		return PaintCaption(hWnd,FRAME);
	}
	case WM_NCACTIVATE://stuff for custom frame
	{
		return TRUE;
	}
	case WM_NCHITTEST://stuff for custom frame
	{
		return HitTestNCA(hWnd, wParam, lParam,FRAME);
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
#ifdef _DEBUG
	case WM_TIMER: {
		//Showing current fps counter for the Veil
		KillTimer(hWnd, 1);
		SetWindowText(GetDlgItem(hWnd, SCV_SETTINGS_UPDATE_COUNTER_TEXT), std::to_wstring(numberOfUpdates).c_str());
		numberOfUpdates = 0;
		SetTimer(hWnd, 1, 1000, NULL);
		break;
	}
	case SCV_SETTINGS_UPDATE_COUNTER:
	{
#if MS_UPDATE
		float val = (float)wParam;
		SetWindowText(GetDlgItem(hWnd, SCV_UPDATE_COUNTER_TEXT), std::to_wstring(val).c_str());
#else 
		numberOfUpdates++;
#endif
		break;
	}
#endif
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
			SETTINGS previous_settings = *CurrentValidSettings;

			//TODO(fran): some system for executing the new changes
			//first check what changed, and based on that generate a list of actions to execute
			SaveCurrentValidSettings(hWnd, CurrentValidSettings);

			if (previous_settings.hotkey.mods != CurrentValidSettings->hotkey.mods || previous_settings.hotkey.vk != CurrentValidSettings->hotkey.vk) {
				BOOL hotkey_applied = Settings_ApplyHotkey(GetWindow(hWnd, GW_OWNER), hWnd, CurrentValidSettings);
				//IMPORTANT TODO(fran): standardize what to do on hotkey failure,
				//option 1: we NULL the variables on CurrentValidSettings and tell the hotkey control to delete it's text after a couple seconds of showing it red
				//option 2: try to apply the old hotkey and if it succeedes tell the hotkey control to show the old one the next time
			}

			if (previous_settings.show_tooltips != CurrentValidSettings->show_tooltips)
				TOOLTIP_REPO::Instance().ActivateTooltips(CurrentValidSettings->show_tooltips);
			if (previous_settings.show_tray_icon != CurrentValidSettings->show_tray_icon) {
				if (CurrentValidSettings->show_tray_icon) SendMessage(GetWindow(hWnd, GW_OWNER), SCV_MANAGER_MODIFY_TRAY, TRUE, 0);
				else SendMessage(GetWindow(hWnd, GW_OWNER), SCV_MANAGER_MODIFY_TRAY, FALSE, 0);
			}
			if (previous_settings.language != CurrentValidSettings->language) {
				LCID ret = LANGUAGE_MANAGER::Instance().ChangeLanguage(CurrentValidSettings->language);
				TRAY_HANDLER::Instance().UpdateTrayTips();
			}

			if (previous_settings.start_with_windows != CurrentValidSettings->start_with_windows) {
				if (CurrentValidSettings->start_with_windows) {
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
		//Needed to paint the list that comes out of comboboxes
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
	case WM_SHOWWINDOW:
	{
		if ((BOOL)wParam) {
#if 1	//Show Settings in the middle of the parent
			RECT rec; GetWindowRect(GetWindow(hWnd, GW_OWNER), &rec);
			RECT rec2; GetWindowRect(hWnd, &rec2);
			//TODO(fran):check if settings window is already visible in which case do not change its position
			HMONITOR current_monitor = MonitorFromWindow(GetWindow(hWnd, GW_OWNER), MONITOR_DEFAULTTONEAREST);
			MONITORINFO mon_info; mon_info.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(current_monitor, &mon_info);
			//TODO(fran): fix for multi-monitor, also should we fix for left and right?, finally this isnt really necesary, and depending on
			// the multimon setup the user has it's not even necessary
			int desiredYPos = rec.top + RECTHEIGHT(rec) / 4;
			if (mon_info.rcWork.bottom < (desiredYPos + RECTHEIGHT(rec2))) {
				desiredYPos -= (desiredYPos + RECTHEIGHT(rec2) - mon_info.rcWork.bottom);
			}
			SetWindowPos(hWnd, NULL, rec.left + RECTWIDTH(rec) / 2 - RECTWIDTH(rec2) / 2, desiredYPos
				, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
#else	//Settings on the right of the parent
			RECT mgr_rc; GetWindowRect(GetWindow(hWnd, GW_OWNER), &mgr_rc);
			RECT set_rc; GetWindowRect(hWnd, &set_rc);

			POINT set_pos;
			set_pos.y = mgr_rc.top;
			set_pos.x = mgr_rc.right + max(1, RECTWIDTH(mgr_rc)*.03);

			SetWindowPos(hWnd, NULL, set_pos.x, set_pos.y
				, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
#endif
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_CLOSE:
	{
		ShowWindow(hWnd, SW_HIDE);//TODO(fran): is this still needed?
		RestoreCurrentValidSettingsToControls(hWnd, *CurrentValidSettings);

		return 0;
	}
	case WM_DESTROY:
	{
		//SETTINGS* already contains all the updated information, we can exit without any problems

		DeleteObject(settings_font);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
