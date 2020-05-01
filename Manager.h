#pragma once

#include <Windows.h>
#include "Settings.h"
#include "MacroStandard.h"
#include <string>
#include <map>

/// <summary>
/// Identifies messages sent from the manager's tray icon
/// </summary>
#define SCV_MANAGER_TRAY (SCV_MANAGER_FIRST_MESSAGE+1)

/// <summary>
/// Signals the manager to switch the Veil's on-off state
/// <para>wParam = unused</para>
/// <para>lParam = unused</para>
/// </summary>
#define SCV_MANAGER_TURN_ON_OFF (SCV_MANAGER_FIRST_MESSAGE+2)

#define SCV_MANAGER_TOOLTIP_OPACITY_SLIDER (SCV_MANAGER_FIRST_MESSAGE+3)
/// <summary>
/// Message to send to request a change in the tray status
/// <para>wParam = TRUE to create the tray, FALSE to destroy it</para>
/// <para>lParam = unused</para>
/// </summary>
#define SCV_MANAGER_MODIFY_TRAY (SCV_MANAGER_FIRST_MESSAGE+4)

#define MGR_SLIDER_MAX 99
#define MGR_SLIDER_MIN 0
#define MGR_SLIDER_DANGEROUS 90
#define MGR_SLIDER_SAFE 80


/// /// <summary>
/// All the information the manager needs to start up
/// </summary>
struct MANAGER {
	
	#define SCV_FOREACH_MANAGER_MEMBER(STRUCT_MEMBER) \
			STRUCT_MEMBER(BOOL, is_turned_on, TRUE) \
			STRUCT_MEMBER(int, slider_threshold_pos, 80) \
			STRUCT_MEMBER(int, slider_opacity_pos, 10)   \
			STRUCT_MEMBER(POINT,previous_manager_position,{LONG_MAX SCV_COMMA LONG_MAX})\
			STRUCT_MEMBER(SIZE, previous_screen_size, { LONG_MAX SCV_COMMA LONG_MAX })	\

	SCV_FOREACH_MANAGER_MEMBER(SCV_GENERATE_STRUCT_MEMBER);

	//is_turned_on: Tells at any time if the veil is on or off
	//previous_screen_size: To check that screen size didnt change and therefore the position of the manager will be good, otherwise do not use previous_manager_position

	//TODO(fran): move out of SETTINGS struct? It's pretty big: size = number of elements * 4(size of pointer)
	/// <summary>
	/// All the variable names contained in the struct, in string form
	/// </summary>
	constexpr static const wchar_t* MANAGER_STRING[] = { //const is not required but I feel it's clearer
			SCV_FOREACH_MANAGER_MEMBER(SCV_GENERATE_STRING_FROM_STRUCT_MEMBER)
	};

	/// <summary>
	/// Converts a SETTINGS objetc into a proper string map with key value pairs, where the key is the variable name as a string
	/// <para>and the value is the variable value also converted to string</para>
	/// </summary>
	std::map<const std::wstring, std::wstring> to_wstring_map() const {
		std::map<const std::wstring, std::wstring> stringed_struct;
		int i = -1;
#pragma warning(suppress : 4002)
		SCV_FOREACH_MANAGER_MEMBER(i++; stringed_struct[MANAGER_STRING[i]] = SCV_SERIALIZE_STRUCT_MEMBER);
		return stringed_struct;
	}

	//INFO IMPORTANT: First load the structs, then you create the objects that need those structs, this way if one structs needs others to be created you
	//can handle all that BEFORE creating the object that uses the struct

	//TODO(fran): make this a constructor?
	MANAGER& from_wstring_map(std::map<const std::wstring, std::wstring> stringed_struct,const SETTINGS& settings /*This will need to be taken out if we make a superclass*/) {
		
		std::wstring potential_string_value;
		int i = 0;
#pragma warning(suppress : 4002)
		SCV_FOREACH_MANAGER_MEMBER(potential_string_value = stringed_struct[MANAGER_STRING[i]]; i++; SCV_DESERIALIZE_STRUCT_MEMBER); //this ; is not needed but is good to maintain syntax
		
		switch (settings.show_veil_on_startup) {
		case VEIL_ON_STARTUP::YES: //TODO(fran): using VEIL_ON_STARTUP (create namespaces)
			is_turned_on = TRUE;
			break;
		case VEIL_ON_STARTUP::NO:
			is_turned_on = FALSE;
			break;
		case VEIL_ON_STARTUP::REMEMBER_LAST_STATE:
		default:
			//Do nothing, just stay with what the variable already has
			break;
		}
		SanitizeSlider(slider_threshold_pos, settings.reduce_dangerous_slider_values);
		SanitizeSlider(slider_opacity_pos, settings.reduce_dangerous_slider_values);

		return *this;
	}

private:
	/// <summary>
	/// Makes sure the slider value is between bounds and fixes it if not
	/// </summary>
	inline void SanitizeSlider(int& val, bool reduce_dangerous) {
		if (val > MGR_SLIDER_MAX || val < MGR_SLIDER_MIN)
			val = (MGR_SLIDER_MAX - MGR_SLIDER_MIN) / 2;
		if (reduce_dangerous && val > MGR_SLIDER_DANGEROUS) val = MGR_SLIDER_SAFE;
	}
};

/// <summary>
/// Window procedure for the Manager window
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK MgrProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
