#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include "utils.cpp"

#define SCV_HOTKEY_REG_FAILED (WM_USER+6)
#define SCV_HOTKEY_REG_SUCCESS (WM_USER+7)
#define SCV_HOTKEY_RESET_TEXT (WM_USER+25) //botched

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

	void Set_BackgroundColor(COLORREF newColor) {
		if (Background) DeleteObject(Background);
		Background = CreateSolidBrush(newColor);
	}

	void Set_HighlightColor(COLORREF newColor) {
		if (Highlight) DeleteObject(Highlight);
		Highlight = CreateSolidBrush(newColor);
	}

	void Set_PushColor(COLORREF newColor){
		if (BackgroundPush) DeleteObject(BackgroundPush);
		BackgroundPush = CreateSolidBrush(newColor);
	}

	void Set_MouseoverColor(COLORREF newColor) {
		if (BackgroundMouseover) DeleteObject(BackgroundMouseover);
		BackgroundMouseover = CreateSolidBrush(newColor);
	}

	void Set_CaptionBackgroundColor(COLORREF newColor) {
		if (CaptionBackground) DeleteObject(CaptionBackground);
		CaptionBackground = CreateSolidBrush(newColor);
	}

	//INFO: to use this functions pass the ControlProcedures object pointer thorugh the last parameter of setwindowsubclass

	static LRESULT CALLBACK ButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK ComboProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK CheckboxProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK CaptionButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SecretButtonProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK HotkeyProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

private:
	ControlProcedures();
	~ControlProcedures();

	HBRUSH BackgroundPush=NULL;
	HBRUSH BackgroundMouseover=NULL;

	HBRUSH Background=NULL;
	HBRUSH Highlight = NULL;

	HBRUSH CaptionBackground = NULL;
};

