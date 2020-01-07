#include "TRAY_HANDLER.h"
#include "LANGUAGE_MANAGER.h"


BOOL TRAY_HANDLER::CreateTrayIcon(HWND hwnd, UINT uID, HICON hicon, UINT uCallbackMessage, WORD messageID)
{
	NOTIFYICONDATA notification;
	notification.cbSize = sizeof(NOTIFYICONDATA);//TODO(fran): this is not quite the way
	notification.hWnd = hwnd;
	notification.uID = uID;
	notification.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;//TODO(fran): check this
	notification.uCallbackMessage = uCallbackMessage;
	notification.hIcon = hicon;
	notification.dwState = NIS_SHAREDICON;//TODO(fran): check this
	notification.dwStateMask = NIS_SHAREDICON;
	notification.szInfo[0] = 0;
	notification.uVersion = NOTIFYICON_VERSION_4;//INFO(fran): this changes the message format, but not when nif_showtip is enabled, I think
	notification.szInfoTitle[0] = 0;
	notification.dwInfoFlags = NIIF_NONE;
	//notification.guidItem = ;
	notification.hBalloonIcon = NULL;
	StringCchCopy(notification.szTip, ARRAYSIZE(notification.szTip), RCS(messageID));//INFO: msg MUST be less than 128 chars
	BOOL ret = Shell_NotifyIcon(NIM_ADD, &notification);
	if (ret)
		this->TrayElements[std::make_pair(hwnd,uID)] = std::make_pair(notification,messageID);//TODO(fran): should we check whether there is an element in that key already?
	return ret;
}

BOOL TRAY_HANDLER::DestroyTrayIcon(HWND hwnd, UINT uID)
{
	try {
		std::pair<NOTIFYICONDATA,WORD> notif_msg = this->TrayElements.at(std::make_pair(hwnd,uID));
		BOOL ret = Shell_NotifyIcon(NIM_DELETE, &notif_msg.first);
		DestroyIcon(notif_msg.first.hIcon);
		this->TrayElements.erase(std::make_pair(hwnd, uID));
		return ret;
	}
	catch (const std::out_of_range& oor) { return FALSE; }
	return FALSE;
}

BOOL TRAY_HANDLER::UpdateTrayTips()
{
	for (auto const& hwnd_msg : this->TrayElements)
		this->UpdateTrayTip(hwnd_msg.second.first, hwnd_msg.second.second);
	return TRUE;
}

BOOL TRAY_HANDLER::UpdateTrayTip(NOTIFYICONDATA notification, WORD messageID)
{
	StringCchCopy(notification.szTip, ARRAYSIZE(notification.szTip), RCS(messageID));
	Shell_NotifyIcon(NIM_MODIFY, &notification);
	return 0;
}

TRAY_HANDLER::TRAY_HANDLER()
{
}


TRAY_HANDLER::~TRAY_HANDLER()
{
}

