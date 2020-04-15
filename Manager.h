#pragma once

#include <Windows.h>
#include "Settings.h"

//TODO(fran): indicate what each defines refers to
#define SCV_MANAGER_FIRST_MESSAGE (WM_USER+0)

#define SCV_MANAGER_TRAY (SCV_MANAGER_FIRST_MESSAGE+1)
#define SCV_MANAGER_TURN_ON_OFF (SCV_MANAGER_FIRST_MESSAGE+2)
#define SCV_MANAGER_THRESHOLD_TITLE (SCV_MANAGER_FIRST_MESSAGE+3)
#define SCV_MANAGER_TOOLTIP_THRESHOLD_SLIDER (SCV_MANAGER_FIRST_MESSAGE+4)
#define SCV_MANAGER_SECRET_ABOUT (SCV_MANAGER_FIRST_MESSAGE+5)
#define SCV_MANAGER_SETTINGS (SCV_MANAGER_FIRST_MESSAGE+6)
#define SCV_MANAGER_TOOLTIP_OPACITY_SLIDER (SCV_MANAGER_FIRST_MESSAGE+7)
/// <summary>
/// Message to send to request a change in the tray status
/// <para>wParam = TRUE to create the tray, FALSE to destroy it</para>
/// <para>lParam = unused</para>
/// </summary>
#define SCV_MANAGER_MODIFY_TRAY (SCV_MANAGER_FIRST_MESSAGE+8)//TODO(fran): no need to have two defines, could send true or false through param
#define SCV_MANAGER_LANG_DYNAMIC_UPDATE (SCV_MANAGER_FIRST_MESSAGE+9)
#define SCV_MANAGER_UPDATE_TEXT_TURN_ON_OFF (SCV_MANAGER_FIRST_MESSAGE+10)
#define SCV_MANAGER_UPDATE_THRESHOLD_OPACITY (SCV_MANAGER_FIRST_MESSAGE+11)
#define SCV_MANAGER_OPACITY_TITLE (SCV_MANAGER_FIRST_MESSAGE+12)


#define SCV_VEIL_FIRST_MESSAGE (WM_USER+500)

#define SCV_VEIL_SHOW_MGR (SCV_VEIL_FIRST_MESSAGE+10)

#define SLIDER_MAX 99
#define SLIDER_MIN 0
#define SLIDER_DANGEROUS 90
#define SLIDER_SAFE 80


/// <summary>
/// All the keys to associate with the values of the MANAGER struct
/// </summary>
const struct MANAGER_TEXT {
	std::wstring is_turned_on = L"is_turned_on";
	std::wstring slider_threshold_pos = L"slider_threshold_pos";
	std::wstring slider_opacity_pos = L"slider_opacity_pos";
	std::wstring previous_manager_position_x = L"manager_pos_x";
	std::wstring previous_manager_position_y = L"manager_pos_y";
	std::wstring previous_screen_size_x = L"screen_size_x";
	std::wstring previous_screen_size_y = L"screen_size_y";
};

/// /// <summary>
/// All the information the manager needs to start up
/// </summary>
struct MANAGER {
	BOOL is_turned_on				= TRUE; // Tells at any time if the veil is on or off
	int slider_threshold_pos		= 80;
	int slider_opacity_pos			= 10;
	POINT previous_manager_position	= { LONG_MAX , LONG_MAX };
	SIZE previous_screen_size		= { LONG_MAX, LONG_MAX };//To check that screen size didnt change and therefore the position of the manager will be good, otherwise do not use previous_manager_position

	/// <summary>
	/// Converts a SETTINGS objetc into a proper string map with key value pairs, where the key is the variable name as a string
	/// <para>and the value is the variable value also converted to string</para>
	/// </summary>
	std::map<const std::wstring, std::wstring> to_wstring_map() const {
		const MANAGER_TEXT var_in_text;
		std::map<const std::wstring, std::wstring> stringed_struct;

		//TODO(fran): MACROSSS
		stringed_struct[var_in_text.is_turned_on] = std::to_wstring(is_turned_on);
		stringed_struct[var_in_text.slider_threshold_pos] = std::to_wstring(slider_threshold_pos);
		stringed_struct[var_in_text.slider_opacity_pos] = std::to_wstring(slider_opacity_pos);
		stringed_struct[var_in_text.previous_manager_position_x] = std::to_wstring(previous_manager_position.x);
		stringed_struct[var_in_text.previous_manager_position_y] = std::to_wstring(previous_manager_position.y);
		stringed_struct[var_in_text.previous_screen_size_x] = std::to_wstring(previous_screen_size.cx);
		stringed_struct[var_in_text.previous_screen_size_y] = std::to_wstring(previous_screen_size.cy);

		return std::move(stringed_struct);
	}

	//INFO IMPORTANT: First load the structs, then you create the objects that need those structs, this way if one structs needs others to be created you
	//can handle all that BEFORE creating the object that uses the struct

	//TODO(fran): make this a constructor?
	MANAGER& from_wstring_map(std::map<const std::wstring, std::wstring> stringed_struct,const SETTINGS& settings /*This will need to be taken out if we make a superclass*/) {
		MANAGER_TEXT var_in_text;
		std::wstring value;
		//TODO(fran): MACROS
		try { 
			
			switch (settings.show_veil_on_startup) {
			case VEIL_ON_STARTUP::YES: //TODO(fran): using VEIL_ON_STARTUP (create namespaces)
				is_turned_on = TRUE;
				break;
			case VEIL_ON_STARTUP::NO:
				is_turned_on = FALSE;
				break;
			case VEIL_ON_STARTUP::REMEMBER_LAST_STATE:
			default:
				value = stringed_struct.at(var_in_text.is_turned_on);
				is_turned_on = stoi(value);
				break;
			}

		} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		try { value = stringed_struct.at(var_in_text.slider_threshold_pos); slider_threshold_pos = SanitizeSlider(stoi(value),settings.reduce_dangerous_slider_values);} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		try { value = stringed_struct.at(var_in_text.slider_opacity_pos); slider_opacity_pos = SanitizeSlider(stoi(value), settings.reduce_dangerous_slider_values);} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		try { value = stringed_struct.at(var_in_text.previous_manager_position_x); previous_manager_position.x = stol(value);} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		try { value = stringed_struct.at(var_in_text.previous_manager_position_y); previous_manager_position.y = stol(value);} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		try { value = stringed_struct.at(var_in_text.previous_screen_size_x); previous_screen_size.cx = stol(value);} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		try { value = stringed_struct.at(var_in_text.previous_screen_size_y); previous_screen_size.cy = stol(value);} catch(std::out_of_range& o){ } catch(std::invalid_argument& i){ }
		return *this;
	}

private:
	/// <summary>
	/// Makes sure the slider value is between bounds and fixes it if not
	/// </summary>
	inline int SanitizeSlider(int val, bool reduce_dangerous) {
		if (val > SLIDER_MAX || val < SLIDER_MIN)
			val = (SLIDER_MAX - SLIDER_MIN) / 2;
		if (reduce_dangerous && val > SLIDER_DANGEROUS) val = SLIDER_SAFE;
		return val;
	}
};

/// <summary>
/// The Veil's window procedure
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK VeilProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/// <summary>
/// Window procedure for the Manager window
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK MgrProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);