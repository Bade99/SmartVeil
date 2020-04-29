#include "Veil.h"
#include "Common.h"

LRESULT CALLBACK VeilProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case SCV_VEIL_SHOW_MGR://Only used in case of multiple instances so the new one can tell the one already running to show the manager
	{
		ShowWindow(KNOWN_WINDOWS.mgr, SW_SHOW);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
