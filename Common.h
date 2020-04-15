#pragma once
#include <Windows.h>

//------------------------------------------------------------------------------------------------------------
//This file defines functions and definitions that are specific to this application and are directly dependent
//------------------------------------------------------------------------------------------------------------

///First index from which windows messages will be sent
#define SCV_COMMON_FIRST_MESSAGE (WM_USER+2000)

struct CUSTOM_FRAME { // Defines the sizes for the custom window frame
	int caption_height;
	int left_border;
	int right_border;
	int bottom_border;
	HBRUSH caption_bk_brush;
	COLORREF caption_text_color; //TODO(fran): color information should be sent straight to paint caption since each window could have slightly different caption colors depending on its state
	HFONT caption_font;
	int logo_icon;
};
/// <summary>
/// Struct object that everyone can use to define and know about the frame for any window, since it MUST the same for everyone
/// </summary>
extern CUSTOM_FRAME FRAME; //defined in Main.cpp

/// <summary>
/// Contains all the windows needed for the program
/// </summary>
struct _KNOWN_WINDOWS {//TODO(fran): look for a better solution
	HWND veil;
	HWND mgr;
	HWND settings;
};
extern _KNOWN_WINDOWS KNOWN_WINDOWS; //defined in Main.cpp

//Messages that will come from custom frames
#define SCV_CUSTOMFRAME_CLOSE (SCV_COMMON_FIRST_MESSAGE+1)
#define SCV_CUSTOMFRAME_MINIMIZE (SCV_COMMON_FIRST_MESSAGE+2)

/// <summary>
/// Use instead of GetClientRect. Can and probably will return left and top values that are non zero since now we draw our own custom frame and the whole window rect is ours
/// </summary>
BOOL GetMyClientRect(HWND hwnd, const CUSTOM_FRAME& frame, RECT* rc);

/// <summary>
/// Creates the application wide font in the desired height
/// </summary>
/// <param name="height">Must be a negative value, eg 10 -> -10</param>
/// <returns>The new font or NULL if failed</returns>
HFONT CreateMyFont(LONG height);

/// <summary>
/// Paints the title area of a window (its caption area)
/// </summary>
/// <param name="hWnd"></param>
/// <returns></returns>
LRESULT PaintCaption(HWND hWnd, CUSTOM_FRAME frame);

/// <summary>
/// Hit test the window frame for resizing and moving.
/// </summary>
/// <param name="hWnd"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam,CUSTOM_FRAME frame);