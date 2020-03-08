#include "TOOLTIP_REPO.h"



TOOLTIP_REPO::TOOLTIP_REPO()
{
}


TOOLTIP_REPO::~TOOLTIP_REPO()
{
}

HINSTANCE TOOLTIP_REPO::SetHInstance(HINSTANCE hInst)
{
	HINSTANCE oldHInst = this->hInstance;
	this->hInstance = hInst;
	return oldHInst;
}


void TOOLTIP_REPO::ActivateTooltips(BOOL activate)
{
	this->active = activate;
	for (auto const& tip : this->tooltips) 
		SendMessage(tip, TTM_ACTIVATE, activate, 0);
}

HWND TOOLTIP_REPO::CreateToolTip(HWND hWnd, WORD messageID) {
	
	// Create the tooltip. 
	HWND hwndTip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP //| TTS_BALLOON
		, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hWnd, NULL, this->hInstance, NULL);

	if (!hwndTip) return (HWND)NULL;

	// Associate the tooltip with the tool.
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(TOOLINFO);
	toolInfo.hwnd = NULL;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hWnd;
	toolInfo.lpszText = (LPWSTR)MAKELPARAM(messageID, 0);
	toolInfo.hinst = this->hInstance;

	//GetClientRect(hwndTool, &toolInfo.rect);
	//SendMessageW(hwndTip, TTM_ACTIVATE, TRUE, 0);

	if (!SendMessageW(hwndTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&toolInfo)) { //Will add the Tool Tip on Control
		DestroyWindow(hwndTip);
		return (HWND)NULL;
	}
	SendMessage(hwndTip, TTM_ACTIVATE, this->active, 0);
	this->AddTooltip(hwndTip);
	return hwndTip;
}

HWND TOOLTIP_REPO::CreateToolTipForRect(HWND hwndParent, WORD messageID)
//INFO: Im really just creating a tooltip for the parent and assigning it a rect to work with
{
	// Create a tooltip.
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP //| TTS_BALLOON
		, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, this->hInstance, NULL);

	//SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set up "tool" information. In this case, the "tool" is the entire parent window.

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwndParent;
	ti.hinst = this->hInstance;
	ti.lpszText = (LPWSTR)MAKELPARAM(messageID, 0);

	GetClientRect(hwndParent, &ti.rect);

	// Associate the tooltip with the "tool" window.
	if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti)) {
		//MessageBox(hwndParent, TEXT("Failed to add tooltip"), L"Smart Veil Error", MB_OK);
		DestroyWindow(hwndTT);
		return (HWND)NULL;
	}
	SendMessage(hwndTT, TTM_ACTIVATE, this->active, 0);
	this->AddTooltip(hwndTT);
	return hwndTT;
}

HWND TOOLTIP_REPO::CreateToolTipForRect(HWND hwndParent, HWND hwndTool, WORD messageID)
//INFO: Im really just creating a tooltip for the parent and resizing it to the position & size of the child
{
	// Create a tooltip.
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP //| TTS_BALLOON
		, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, this->hInstance, NULL);

	//SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set up "tool" information. In this case, the "tool" is the entire parent window.

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwndParent;
	ti.hinst = this->hInstance;
	ti.lpszText = (LPWSTR)MAKELPARAM(messageID, 0);

	GetClientRect(hwndTool, &ti.rect);
	MapWindowPoints(hwndTool, hwndParent, (LPPOINT)&ti.rect, 2);
	//	ti.rect.right /= 2;

		// Associate the tooltip with the "tool" window.
	if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti)) {
		//MessageBox(hwndParent, TEXT("Failed to add tooltip"), L"Smart Veil Error", MB_OK);
		DestroyWindow(hwndTT);
		return (HWND)NULL;
	}
	SendMessage(hwndTT, TTM_ACTIVATE, this->active, 0);
	this->AddTooltip(hwndTT);
	return hwndTT;
}

void TOOLTIP_REPO::AddTooltip(HWND tooltip)
{
	if (!tooltip) return;
	tooltips.push_back(tooltip);
}
