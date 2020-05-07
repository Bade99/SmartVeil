#pragma once
#include <Windows.h>
#include "utils.cpp"

#define SCV_CONTROLPROC_FIRST_MESSAGE (WM_USER+4000)

#define SCV_HOTKEY_REG_FAILED (SCV_CONTROLPROC_FIRST_MESSAGE+1)
#define SCV_HOTKEY_REG_SUCCESS (SCV_CONTROLPROC_FIRST_MESSAGE+2)
#define SCV_HOTKEY_RESET_TEXT (SCV_CONTROLPROC_FIRST_MESSAGE+3) //botched

class ControlProcedures
{
public:
	static ControlProcedures& Instance() //https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
	{
		static ControlProcedures instance; // Guaranteed to be destroyed.
									  // Instantiated on first use.
		return instance;
	}

	ControlProcedures(ControlProcedures const&) = delete;
	void operator=(ControlProcedures const&) = delete;

	/// <summary>
	/// Set the id of the minimize icon for the title area
	/// </summary>
	/// <param name="ID">This is used for MAKEINTRESOURCE(ID)</param>
	void Set_CaptionMinimizeIconID(LONG ID) {
		CaptionMinimizeIconID = ID;
	}

	/// <summary>
	/// Set the id of the maximize icon for the title area
	/// </summary>
	/// <param name="ID">This is used for MAKEINTRESOURCE(ID)</param>
	void Set_CaptionCloseIconID(LONG ID) {
		CaptionCloseIconID = ID;
	}

	/// <summary>
	/// Set the id of the dropdown icon of the comboboxes
	/// </summary>
	/// <param name="ID">This is used for MAKEINTRESOURCE(ID)</param>
	void Set_ComboIconID(LONG ID) {
		ComboIconID = ID;
	}

	/// <summary>
	/// Set the id of the tick mark icon of the checkboxes
	/// </summary>
	/// <param name="ID">This is used for MAKEINTRESOURCE(ID)</param>
	void Set_CheckboxIconID(LONG ID) {
		CheckboxIconID = ID;
	}

	/// <summary>
	/// Set the color of the background of controls
	/// </summary>
	/// <param name="newColor"></param>
	void Set_BackgroundColor(COLORREF newColor) {
		if (Background) DeleteObject(Background);
		Background = CreateSolidBrush(newColor);
	}

	/// <summary>
	/// Set the color of text and borders
	/// </summary>
	/// <param name="newColor"></param>
	void Set_HighlightColor(COLORREF newColor) {
		if (Highlight) DeleteObject(Highlight);
		Highlight = CreateSolidBrush(newColor);
	}

	/// <summary>
	/// Set the color of the control when it has been pushed
	/// </summary>
	/// <param name="newColor"></param>
	void Set_PushColor(COLORREF newColor){
		if (BackgroundPush) DeleteObject(BackgroundPush);
		BackgroundPush = CreateSolidBrush(newColor);
	}

	/// <summary>
	/// Set the color of the control when the user moves the mouse over it
	/// </summary>
	/// <param name="newColor"></param>
	void Set_MouseoverColor(COLORREF newColor) {
		if (BackgroundMouseover) DeleteObject(BackgroundMouseover);
		BackgroundMouseover = CreateSolidBrush(newColor);
	}

	/// <summary>
	/// Set the color of the background for the title area
	/// </summary>
	/// <param name="newColor"></param>
	void Set_CaptionBackgroundColor(COLORREF newColor) {
		if (CaptionBackground) DeleteObject(CaptionBackground);
		CaptionBackground = CreateSolidBrush(newColor);
	}

	HBRUSH Get_BackgroundBrush() {
		return Background;
	}

	HBRUSH Get_BackgroundPushBrush() {
		return BackgroundPush;
	}

	HBRUSH Get_BackgroundMouseoverBrush() {
		return BackgroundMouseover;
	}

	HBRUSH Get_HighlightBrush() {
		return Highlight;
	}

	HBRUSH Get_CaptionBackgroundBrush() {
		return CaptionBackground;
	}

	COLORREF Get_BackgroundColor() {
		return ColorFromBrush(Background);
	}

	COLORREF Get_HighlightColor() {
		return ColorFromBrush(Highlight);
	}

	COLORREF Get_BackgroundPushColor() {
		return ColorFromBrush(BackgroundPush);
	}

	COLORREF Get_BackgroundMouseoverColor() {
		return ColorFromBrush(BackgroundMouseover);
	}

	COLORREF Get_CaptionBackgroundColor() {
		return ColorFromBrush(CaptionBackground);
	}

	//Subclassing functions:

	//Usage examples: 
	
	//Subclassing: SetWindowSubclass(ButtonHWND, ControlProcedures::Instance().ButtonProc, 0, (DWORD_PTR)&ControlProcedures::Instance());
	
	//Changing color of combobox's list:
	//case WM_CTLCOLORLISTBOX:
	//{
	//	HDC comboboxDC = (HDC)wParam;
	//	SetTextColor(comboboxDC, ControlProcedures::Instance().Get_HighlightColor());
	//	SetBkColor(comboboxDC, ControlProcedures::Instance().Get_BackgroundColor());
	//	return (INT_PTR)ControlProcedures::Instance().Get_BackgroundBrush();
	//}

	//Anything not commented about this procedures and their corresponding controls means that it works exactly the same as the default Windows ones

	/// <summary>
	/// Creates buttons with a border and vertically and horizontally centered text/icon (not both at the same time)
	/// <para>To assign an icon: SendMessage(ButtonHWND, BM_SETIMAGE, IMAGE_ICON, "ICON_ID"); Note that this control does not support bitmaps</para>
	/// <para>ICON_ID will be used with MAKEINTRESOURCE(ICON_ID)</para>
	/// <para>Your HWND must have its hInstance parameter set if your button has an icon</para>
	/// <para>Supports coloring using the functions provided by this "API"</para>
	/// <para>See ControlProcedures.h for usage example</para>
	/// </summary>
	static LRESULT CALLBACK ButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	/// <summary>
	/// Creates comboboxes with a border and vertically centered text that is left aligned
	/// <para>Your HWND must have its hInstance parameter set</para>
	/// <para>The colors of the list that comes out of the combobox when pressed can be changed by managing the WM_CTLCOLORLISTBOX message on the parent</para>
	/// <para>You need to set the dropdown icon of the comboboxes by calling Set_ComboIconID()</para>
	/// <para>Supports coloring using the functions provided by this "API"</para>
	/// <para>See ControlProcedures.h for usage example</para>
	/// </summary>
	static LRESULT CALLBACK ComboProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	/// <summary>
	/// Creates a left sided checkbox with text on its right
	/// <para>Your HWND must have its hInstance parameter set</para>
	/// <para>Only works if you have set the BS_AUTOCHECKBOX style</para>
	/// <para>You need to set the dropdown icon of the comboboxes by calling Set_CheckboxIconID()</para>
	/// <para>Supports coloring using the functions provided by this "API"</para>
	/// <para>See ControlProcedures.h for usage example</para>
	/// </summary>
	static LRESULT CALLBACK CheckboxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	/// <summary>
	/// This buttons dont have borders and are intended to replace the Minimize and Close ones (no maximize button)
	/// <para>To create a Minimize button add WS_MINIMIZEBOX to your button's styles</para>
	/// <para>To create a Close button add WS_MAXIMIZEBOX to your button's styles (nop, didn't misspell)</para>
	/// <para>You need to set the icons for both buttons by calling Set_CaptionMinimizeIconID() and Set_CaptionCloseIconID()</para>
	/// <para>Your HWND must have its hInstance parameter set</para>
	/// <para>For now you still have to assign them a message id, these controls dont send their respective WM_SYSCOMMAND message</para>
	/// <para>Supports coloring using the functions provided by this "API"</para>
	/// <para>See ControlProcedures.h for usage example</para>
	/// </summary>
	static LRESULT CALLBACK CaptionButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	/// <summary>
	/// An invisible button (uses background color only) that you can still press to send a message
	/// Not much more to this one, I just use it till I find a good place to put those objects
	/// <para>Supports coloring using the functions provided by this "API"</para>
	/// <para>See ControlProcedures.h for usage example</para>
	/// </summary>
	static LRESULT CALLBACK SecretButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	//TODO(fran): work in progress
	static LRESULT CALLBACK HotkeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

private:
	ControlProcedures();
	~ControlProcedures();

	HBRUSH BackgroundPush=NULL;
	HBRUSH BackgroundMouseover=NULL;

	HBRUSH Background=NULL;
	HBRUSH Highlight = NULL;

	HBRUSH CaptionBackground = NULL;

	LONG ComboIconID = NULL;

	LONG CheckboxIconID = NULL;

	LONG CaptionMinimizeIconID = NULL;
	LONG CaptionCloseIconID = NULL;
};

