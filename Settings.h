#pragma once

#include <Windows.h>
#include <string>
#include <map>
#include "Serialization.h"
#include "LANGUAGE_MANAGER.h"
#include "MacroStandard.h" //TODO(fran): should this be in the cpp somehow?

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

//TODO(fran): decide what to do with this serializations, dependency between Settings.h and Serialization.h does not allow for the program to compile
inline std::wstring serialize(VEIL_ON_STARTUP v) {
	return std::to_wstring(v);
}
inline void deserialize(VEIL_ON_STARTUP& v, const std::wstring& s) { try { v = (VEIL_ON_STARTUP)stoi(s); } catch (...) {} }

//HOTKEY (should probably choose a longer name SCV_HOTKEY or SETTINGS_HOTKEY)
inline std::wstring serialize(HOTKEY v) {
	using namespace std::string_literals;
	return L"{"s + std::to_wstring(v.mods) + L","s + std::to_wstring(v.vk) + L"}"s;
}
inline void deserialize(HOTKEY& v, const std::wstring& s) { //TODO(fran): use regex?
	size_t open = 0, comma = 0, close = 0;
	open = s.find(L'{', open);
	comma = s.find(L',', open + 1);
	close = s.find(L'}', comma + 1);
	if (open == std::wstring::npos || comma == std::wstring::npos || close == std::wstring::npos)
		return;
	std::wstring mods = s.substr(open + 1, comma - open);
	std::wstring vk = s.substr(comma + 1, close - comma);
	HOTKEY temp;
	try
	{
		temp.mods = stoul(mods);
		temp.vk = stoul(vk);
		v = temp;
	}
	catch (...) {}
	//Allows anything that contains {number,number} somewhere in its string, eg: }}}}}{51,12}}{}{12,}{ is valid
}




/// <summary>
/// Structure that contains all the current valid values from the Settings window
/// </summary>
struct SETTINGS {

	//------HORRIBLE MACROS ALERT------// I'm open to cleaner solutions to implement reflection while they remain fast and simple

	#define SCV_FOREACH_SETTINGS_MEMBER(STRUCT_MEMBER) \
			STRUCT_MEMBER(HOTKEY, hotkey, { VK_F9 SCV_COMMA MOD_CONTROL }) \
			STRUCT_MEMBER(LANGUAGE_MANAGER::LANGUAGE, language, LANGUAGE_MANAGER::LANGUAGE::ENGLISH)		\
			STRUCT_MEMBER(VEIL_ON_STARTUP, show_veil_on_startup, VEIL_ON_STARTUP::YES) \
			STRUCT_MEMBER(BOOL, start_with_windows, FALSE)		\
			STRUCT_MEMBER(BOOL, show_manager_on_startup, TRUE) \
			STRUCT_MEMBER(BOOL,show_tray_icon,TRUE) \
			STRUCT_MEMBER(BOOL, show_tooltips, TRUE) \
			STRUCT_MEMBER(BOOL, reduce_dangerous_slider_values, TRUE) \
			STRUCT_MEMBER(BOOL, remember_manager_position, FALSE) \
	
	SCV_FOREACH_SETTINGS_MEMBER(SCV_GENERATE_STRUCT_MEMBER);

	//TODO(fran): if start with windows is true we MUST reduce dangerous values, the other boolean will be grayed out and ignored
	// If some startup slider value is bigger than the set limit then it should be reduced to a safer value


	//TODO(fran): move out of SETTINGS struct? It's pretty big: size = number of elements * 4(size of pointer)
	/// <summary>
	/// All the variable names contained in the struct, in string form
	/// </summary>
	constexpr static const wchar_t* SETTINGS_STRING[] = { //const is not required but I feel it's clearer
			SCV_FOREACH_SETTINGS_MEMBER(SCV_GENERATE_STRING_FROM_STRUCT_MEMBER)
	};

	/// <summary>
	/// Converts a SETTINGS objetc into a proper string map with key value pairs, where the key is the variable name as a string
	/// <para>and the value is the variable value also converted to string</para>
	/// </summary>
	/// <param name="settings"></param>
	/// <returns></returns>
	std::map<const std::wstring, std::wstring> to_wstring_map() const { //TODO(fran): should I call it serialize? It's a changing object and needs variable names, I dont know
		
		std::map<const std::wstring, std::wstring> stringed_struct;
		int i = -1;
		SCV_FOREACH_SETTINGS_MEMBER(i++; stringed_struct[SETTINGS_STRING[i]] = SCV_SERIALIZE_STRUCT_MEMBER);
		return stringed_struct;
	}

	//SETTINGS() {};

	SETTINGS& from_wstring_map(std::map<const std::wstring, std::wstring> stringed_struct) { //deserialize //TODO(fran): is it ok to call this a constructor?
		std::wstring potential_string_value;
		int i = 0;
		SCV_FOREACH_SETTINGS_MEMBER(potential_string_value = stringed_struct[SETTINGS_STRING[i]]; i++; SCV_DESERIALIZE_STRUCT_MEMBER); //this ; is not needed but is good to maintain syntax
		return *this;
	}

	//------HORRIBLE MACROS END------//

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