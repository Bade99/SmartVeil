#pragma once
#include <Windows.h>
#include "MacroStandard.h"

/// <summary>
/// Shows the manager window associated with this Veil
/// </summary>
#define SCV_VEIL_SHOW_MGR (SCV_VEIL_FIRST_MESSAGE+1)

/// <summary>
/// The Veil's window procedure
/// </summary>
LRESULT CALLBACK VeilProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);