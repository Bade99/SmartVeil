#pragma once

#include <windows.h>
#include "LANGUAGE_MANAGER.h"
#include <string>
#include "utils.cpp"

enum VEIL_ON_STARTUP {
	YES = 0, NO, REMEMBER_LAST_STATE
};

struct STARTUP_INFO {
	BOOL is_turned_on;
	int slider_threshold_pos;
	int slider_opacity_pos;
	UINT hotkey_modifiers;
	UINT hotkey_virtual_key;
	LANGUAGE_MANAGER::LANGUAGE language;
	POINT previous_manager_position;
	SIZE previous_screen_size;//To check that screen size didnt change and therefore the position of the manager will be good, otherwise do not use previous_manager_position

	BOOL start_with_windows; //if start with windows is true we MUST reduce dangerous values, the other boolean will be grayed out and ignored
	VEIL_ON_STARTUP show_veil_on_startup;
	BOOL show_manager_on_startup;
	BOOL show_tray_icon;
	BOOL show_tooltips;//TODO(fran): could put the hotkey fail/success tooltip on a separate option
	BOOL reduce_dangerous_slider_values;// If some startup slider value is >90% it will reduce it to a safer value, in case the user screwed up
	BOOL remember_manager_position;
};

struct STARTUP_INFO_TEXT {
	std::wstring hotkey_modifiers = L"hotkey_mods";
	std::wstring hotkey_virtual_key = L"hotkey_vk";
	std::wstring is_turned_on = L"is_turned_on";
	std::wstring language = L"lang";
	std::wstring previous_manager_position_x = L"manager_pos_x";
	std::wstring previous_manager_position_y = L"manager_pos_y";
	std::wstring previous_screen_size_x = L"screen_size_x";
	std::wstring previous_screen_size_y = L"screen_size_y";
	std::wstring reduce_dangerous_slider_values = L"reduce_dangerous_sliders";
	std::wstring remember_manager_position = L"remember_manager_pos";
	std::wstring show_manager_on_startup = L"show_manager_on_startup";
	std::wstring show_tooltips = L"show_tooltips";
	std::wstring show_tray_icon = L"show_tray_icon";
	std::wstring show_veil_on_startup = L"show_veil_on_startup";
	std::wstring slider_threshold_pos = L"slider_threshold_pos";
	std::wstring slider_opacity_pos = L"slider_opacity_pos";
	std::wstring start_with_windows = L"start_with_windows";
	wchar_t separator = L'=';
};

struct SETTINGS {
	//UINT hotkey_modifiers;//TODO(fran): put the CurrentValidHotkey here now
	//UINT hotkey_virtual_key;
	LANGUAGE_MANAGER::LANGUAGE language;
	BOOL start_with_windows; //if start with windows is true we MUST reduce dangerous values, the other boolean will be grayed out and ignored
	VEIL_ON_STARTUP show_veil_on_startup;
	BOOL show_manager_on_startup;
	BOOL show_tray_icon;
	BOOL show_tooltips;
	BOOL reduce_dangerous_slider_values;
	BOOL remember_manager_position;
};//this structure will be used to store the current valid settings, so in case the user cancels some change we can rollback

inline void DefaultStartupInfo(STARTUP_INFO &startup_info) {//TODO(fran): this could be a constructor
	startup_info.hotkey_modifiers = MOD_CONTROL;
	startup_info.hotkey_virtual_key = VK_F9;
	startup_info.is_turned_on = TRUE;
	startup_info.language = LANGUAGE_MANAGER::LANGUAGE::ENGLISH;
	startup_info.previous_manager_position.x = LONG_MAX;
	startup_info.previous_manager_position.y = LONG_MAX;
	startup_info.previous_screen_size.cx = LONG_MAX;
	startup_info.previous_screen_size.cy = LONG_MAX;
	startup_info.reduce_dangerous_slider_values = TRUE;
	startup_info.remember_manager_position = FALSE;
	startup_info.show_manager_on_startup = TRUE;
	startup_info.show_tooltips = TRUE;
	startup_info.show_tray_icon = TRUE;
	startup_info.show_veil_on_startup = VEIL_ON_STARTUP::YES;
	startup_info.slider_opacity_pos = 10;
	startup_info.slider_threshold_pos = 80;
	startup_info.start_with_windows = FALSE;
}

inline void SanitizeSliderValue(int &val) {
	//TODO(fran): establish max and min slider values somewhere
	if (val > 99 || val < 0) {//TODO(fran): I think there is no real problem with leaving this hardcoded, the sliders will never change, I hope
		if (val > 99) val = 80;
		else val = 10;
	}
}

#define DANGEROUS_OPACITY_SLIDER_VALUE 90
#define SAFE_OPACITY_SLIDER_VALUE 80

//Fills the info with the data from the string
inline void FillStartupInfo(std::map<std::wstring, std::wstring> info_mapped, STARTUP_INFO &startup_info) {
	STARTUP_INFO_TEXT startup_info_text;
	std::wstring info_str;

	DefaultStartupInfo(startup_info);//TODO(fran): should we default before coming to this function?, then we dont need to put default many times

	//TODO(fran): awful code repetition, we need something more fancy
	try {
		info_str = info_mapped.at(startup_info_text.hotkey_modifiers);
		try {
			startup_info.hotkey_modifiers = stoul(info_str);//TODO(fran): check that on thrown exception the value doenst get changed to garbage
		}
		catch (...) { /*startup_info.hotkey_modifiers = 0;*/ }
	}
	catch (...) {} //INFO: since, for now, we are defaulting everything at the start, we dont need to put the default value in the catch

	try {
		info_str = info_mapped.at(startup_info_text.hotkey_virtual_key);
		try {
			startup_info.hotkey_virtual_key = stoul(info_str);
		}
		catch (...) { /*startup_info.hotkey_virtual_key = 0;*/ }
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.is_turned_on);
		try {
			startup_info.is_turned_on = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.language);
		try {
			startup_info.language = (LANGUAGE_MANAGER::LANGUAGE)stoi(info_str);
			//TODO(fran): check valid values
			if (!LANGUAGE_MANAGER::IsValidLanguage(startup_info.language)) startup_info.language = LANGUAGE_MANAGER::LANGUAGE::ENGLISH;
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.previous_manager_position_x);
		try {
			startup_info.previous_manager_position.x = stol(info_str);
		}
		catch (...) { /*startup_info.previous_manager_position.x = LONG_MAX;*/ }//TODO(fran): establish invalid manager pos value outside of here
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.previous_manager_position_y);
		try {
			startup_info.previous_manager_position.y = stol(info_str);
		}
		catch (...) { /*startup_info.previous_manager_position.y = LONG_MAX;*/ }
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.previous_screen_size_x);
		try {
			startup_info.previous_screen_size.cx = stol(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.previous_screen_size_y);
		try {
			startup_info.previous_screen_size.cy = stol(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.reduce_dangerous_slider_values);//This MUST be done before getting the slider data
		try {
			startup_info.reduce_dangerous_slider_values = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.remember_manager_position);
		try {
			startup_info.remember_manager_position = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.show_manager_on_startup);
		try {
			startup_info.show_manager_on_startup = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.show_tooltips);
		try {
			startup_info.show_tooltips = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.show_tray_icon);
		try {
			startup_info.show_tray_icon = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.show_veil_on_startup);
		try {
			startup_info.show_veil_on_startup = (VEIL_ON_STARTUP)stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.slider_opacity_pos);
		try {
			startup_info.slider_opacity_pos = stoi(info_str);
			SanitizeSliderValue(startup_info.slider_opacity_pos);
			if (startup_info.reduce_dangerous_slider_values) {
				if (startup_info.slider_opacity_pos > DANGEROUS_OPACITY_SLIDER_VALUE) {
					startup_info.slider_opacity_pos = SAFE_OPACITY_SLIDER_VALUE;
				}
			}
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.slider_threshold_pos);
		try {
			startup_info.slider_threshold_pos = stoi(info_str);
			SanitizeSliderValue(startup_info.slider_threshold_pos);
			//TODO(fran): should we push the threshold up to say 10 in case it is below, when reduce_dangerous_slider_values is true?
		}
		catch (...) {}
	}
	catch (...) {}

	try {
		info_str = info_mapped.at(startup_info_text.start_with_windows);
		try {
			startup_info.start_with_windows = stoi(info_str);
		}
		catch (...) {}
	}
	catch (...) {}
}

template <class info_type>
inline void WriteInfoLine(std::wstring &buffer, std::wstring const& key, info_type value, wchar_t const& separator) {
	buffer += key + separator + std::to_wstring(value) + L"\n";
}

inline void FillStartupInfoString(STARTUP_INFO startup_info, std::wstring &info_buf) {
	STARTUP_INFO_TEXT startup_info_text;
	WriteInfoLine(info_buf, startup_info_text.hotkey_modifiers, startup_info.hotkey_modifiers, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.hotkey_virtual_key, startup_info.hotkey_virtual_key, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.is_turned_on, startup_info.is_turned_on, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.language, startup_info.language, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.previous_manager_position_x, startup_info.previous_manager_position.x, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.previous_manager_position_y, startup_info.previous_manager_position.y, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.previous_screen_size_x, startup_info.previous_screen_size.cx, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.previous_screen_size_y, startup_info.previous_screen_size.cy, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.reduce_dangerous_slider_values, startup_info.reduce_dangerous_slider_values, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.remember_manager_position, startup_info.remember_manager_position, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.show_manager_on_startup, startup_info.show_manager_on_startup, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.show_tooltips, startup_info.show_tooltips, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.show_tray_icon, startup_info.show_tray_icon, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.show_veil_on_startup, startup_info.show_veil_on_startup, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.slider_threshold_pos, startup_info.slider_threshold_pos, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.slider_opacity_pos, startup_info.slider_opacity_pos, startup_info_text.separator);
	WriteInfoLine(info_buf, startup_info_text.start_with_windows, startup_info.start_with_windows, startup_info_text.separator);
}

inline void SaveStartupInfo(STARTUP_INFO startup_info, std::wstring directory, std::wstring filename) {//do not put slash at the end of directory string
	std::wstring info_buf;
	FillStartupInfoString(startup_info, info_buf);

	//INFO: the folder MUST have been created previously, CreateFile doesnt do it
	BOOL dir_ret = CreateDirectoryW((LPWSTR)directory.c_str(), NULL);

	if (!dir_ret) {
		Assert(GetLastError() != ERROR_PATH_NOT_FOUND);
	}

	std::wstring full_file_path = directory + L"\\" + filename;
	HANDLE info_file_handle = CreateFileW(full_file_path.c_str(), GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE
		, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (info_file_handle != INVALID_HANDLE_VALUE) {
		DWORD bytes_written;
		BOOL write_res = WriteFile(info_file_handle, (LPWSTR)info_buf.c_str(), info_buf.size() * sizeof(wchar_t), &bytes_written, NULL);
		CloseHandle(info_file_handle);
	}
	else {

	}
	//TODO(fran): do we do something if couldnt create/write the file?
}


//Fills the info with the current data of the program
//TODO(fran): put extra params into a struct or create new struct with startup_info inside (only to use here)
inline void FillStartupInfo(STARTUP_INFO &startup_info, SETTINGS CurrentValidSettings,
					UINT currentvalidhotkeymods,UINT currentvalidhotkeyvk,BOOL isturnedon,
					POINT mgrpos,SIZE virtualscreensize, int thresholdpos, int opacitypos) {
	//TODO(fran): this function should receive the settings hwnd from parameter

	startup_info.hotkey_modifiers = currentvalidhotkeymods;
	startup_info.hotkey_virtual_key = currentvalidhotkeyvk;

	startup_info.is_turned_on = isturnedon;

	startup_info.language = CurrentValidSettings.language;
	startup_info.show_veil_on_startup = CurrentValidSettings.show_veil_on_startup;

	startup_info.previous_manager_position.x = mgrpos.x;
	startup_info.previous_manager_position.y = mgrpos.y;
	startup_info.previous_screen_size.cx = virtualscreensize.cx;
	startup_info.previous_screen_size.cy = virtualscreensize.cy;

	startup_info.reduce_dangerous_slider_values = CurrentValidSettings.reduce_dangerous_slider_values;
	startup_info.remember_manager_position = CurrentValidSettings.remember_manager_position;
	startup_info.show_manager_on_startup = CurrentValidSettings.show_manager_on_startup;
	startup_info.show_tooltips = CurrentValidSettings.show_tooltips;
	startup_info.show_tray_icon = CurrentValidSettings.show_tray_icon;
	startup_info.start_with_windows = CurrentValidSettings.start_with_windows;

	startup_info.slider_threshold_pos = thresholdpos;
	startup_info.slider_opacity_pos = opacitypos;
}