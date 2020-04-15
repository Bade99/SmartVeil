#pragma once

#include <Windows.h>
#include <string>
#include <map>

#include "LANGUAGE_MANAGER.h"

///First index from which windows messages will be sent
#define SCV_SETTINGS_FIRST_MESSAGE (WM_USER+1000)

/// <summary>
/// Message to send to the Settings Window to retrieve its settings
/// <para>wParam = pointer to a SETTINGS structure for storing the current settings</para>
/// <para>lParam = unused</para>
/// </summary>
#define SCV_SETTINGS_GET_SETTINGS (SCV_SETTINGS_FIRST_MESSAGE+1)

/// <summary>
/// Updates the settings' veil update counter
/// <para>wParam = a float that holds the timing data</para>
/// <para>lParam = unused</para>
/// </summary>
#define SCV_SETTINGS_UPDATE_COUNTER (SCV_SETTINGS_FIRST_MESSAGE+2)

#define MS_UPDATE 0 //show veil update rate in milliseconds

/// <summary>
/// Options for the corresponding combobox in the Settings window
/// </summary>
enum VEIL_ON_STARTUP {
	YES = 0, NO, REMEMBER_LAST_STATE
};

/// <summary>
/// Definition for a hotkey, store them in HOTKEY format, NOT the one used by the Hotkey Controls
/// <para>Example:</para>
/// <para>WORD new_hotkey = SendMessage(hotkey_hwnd, HKM_GETHOTKEY, 0, 0);</para>
/// <para>BYTE new_modifs = HotkeyControlModifiersToHotkeyModifiers(HIBYTE(new_hotkey));</para>
/// <para>BYTE new_vk = LOBYTE(new_hotkey);</para>
/// <para>The format of new_modifs and new_vk is the correct one to store here</para>
/// </summary>
struct HOTKEY {
	/// <summary>
	/// Hotkey virtual key
	/// </summary>
	UINT vk;
	/// <summary>
	/// Hotkey modifiers
	/// </summary>
	UINT mods;
};

/// <summary>
/// All the keys to associate with the values of the SETTINGS struct
/// </summary>
const struct SETTINGS_TEXT {
	const std::wstring hotkey_mods = L"hotkey_mods";
	const std::wstring hotkey_vk = L"hotkey_vk";
	const std::wstring language = L"lang";
	const std::wstring show_veil_on_startup = L"show_veil_on_startup";
	const std::wstring start_with_windows = L"start_with_windows";
	const std::wstring show_manager_on_startup = L"show_manager_on_startup";
	const std::wstring show_tray_icon = L"show_tray_icon";
	const std::wstring show_tooltips = L"show_tooltips";
	const std::wstring reduce_dangerous_slider_values = L"reduce_dangerous_sliders";
	const std::wstring remember_manager_position = L"remember_manager_pos";
};

/// <summary>
/// Structure that contains all the current valid values from the Settings window
/// </summary>
struct SETTINGS {
	HOTKEY hotkey							= { VK_F9, MOD_CONTROL };
	LANGUAGE_MANAGER::LANGUAGE language		= LANGUAGE_MANAGER::LANGUAGE::ENGLISH;
	VEIL_ON_STARTUP show_veil_on_startup	= VEIL_ON_STARTUP::YES;
	BOOL start_with_windows					= FALSE; //if start with windows is true we MUST reduce dangerous values, the other boolean will be grayed out and ignored
	BOOL show_manager_on_startup			= TRUE;
	BOOL show_tray_icon						= TRUE;
	BOOL show_tooltips						= TRUE;
	BOOL reduce_dangerous_slider_values		= TRUE; // If some startup slider value is bigger than the set limit then it should be reduced to a safer value
	BOOL remember_manager_position			= FALSE;

	/// <summary>
	/// Converts a SETTINGS objetc into a proper string map with key value pairs, where the key is the variable name as a string
	/// <para>and the value is the variable value also converted to string</para>
	/// </summary>
	/// <param name="settings"></param>
	/// <returns></returns>
	std::map<const std::wstring, std::wstring> to_wstring_map() const {
		const SETTINGS_TEXT var_in_text;
		std::map<const std::wstring, std::wstring> stringed_struct;

		//TODO(fran): I need some hacky macro way of iterating over this----------------------------------------------------------------------------
		stringed_struct[var_in_text.hotkey_mods] = std::to_wstring(hotkey.mods);
		stringed_struct[var_in_text.hotkey_vk] = std::to_wstring(hotkey.vk);
		stringed_struct[var_in_text.language] = std::to_wstring(language);
		stringed_struct[var_in_text.reduce_dangerous_slider_values] = std::to_wstring(reduce_dangerous_slider_values);
		stringed_struct[var_in_text.remember_manager_position] = std::to_wstring(remember_manager_position);
		stringed_struct[var_in_text.show_manager_on_startup] = std::to_wstring(show_manager_on_startup);
		stringed_struct[var_in_text.show_tooltips] = std::to_wstring(show_tooltips);
		stringed_struct[var_in_text.show_tray_icon] = std::to_wstring(show_tray_icon);
		stringed_struct[var_in_text.show_veil_on_startup] = std::to_wstring(show_veil_on_startup);
		stringed_struct[var_in_text.start_with_windows] = std::to_wstring(start_with_windows);

		return std::move(stringed_struct); //TODO(fran): is this neccessary or can I be sure it's moved?
	}

	SETTINGS& from_wstring_map(std::map<const std::wstring, std::wstring> stringed_struct) {
		SETTINGS_TEXT var_in_text;
		std::wstring value;
		//TODO(fran): MACROS
		try { value = stringed_struct.at(var_in_text.hotkey_mods); hotkey.mods = stoul(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.hotkey_vk); hotkey.vk = stoul(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.language); language = LANGUAGE_MANAGER::GetValidLanguage(stoi(value)); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.reduce_dangerous_slider_values); reduce_dangerous_slider_values = stoi(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.remember_manager_position); remember_manager_position = stoi(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.show_manager_on_startup); show_manager_on_startup = stoi(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.show_tooltips); show_tooltips = stoi(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.show_tray_icon); show_tray_icon = stoi(value); } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.show_veil_on_startup); show_veil_on_startup = (VEIL_ON_STARTUP)stoi(value);/*TODO(fran): this should be checked too*/ } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		try { value = stringed_struct.at(var_in_text.start_with_windows); start_with_windows = stoi(value);/*TODO(fran): this should be checked too*/ } catch (std::out_of_range& o) {} catch (std::invalid_argument& i) {}
		return *this;
	}

};

/// <summary>
/// Window procedure for Settings window
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK SettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);