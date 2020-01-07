#pragma once
#include <windows.h>
#include <map>
#include <strsafe.h> //for stringcchcopy
class TRAY_HANDLER
{
public:
	static TRAY_HANDLER& Instance() 
	{
		static TRAY_HANDLER instance;
									 
		return instance;
	}
	TRAY_HANDLER(TRAY_HANDLER const&) = delete;
	void operator=(TRAY_HANDLER const&) = delete;

	//·uID can be any unique number that you assign
	//·uCallbackMessage is the message code that will be sent to the hwnd when some event happens on the tray object
	//·tooltip is the message to show on mouseover
	BOOL CreateTrayIcon(HWND hwnd,UINT uID, HICON hicon,UINT uCallbackMessage, WORD messageID);

	//·Destroys the tray object and the created icon that you sent
	BOOL DestroyTrayIcon(HWND hwnd,UINT uID);

	//·Resets the tip of the tray icons with the string corresponding to messageID + the current language
	BOOL UpdateTrayTips();


private:
	TRAY_HANDLER();
	~TRAY_HANDLER();

	//·Each tray icon (notifyicondata) is mapped by hwnd+uid (into uint)
	std::map<std::pair<HWND, UINT>, std::pair<NOTIFYICONDATA,WORD>> TrayElements;
	
	BOOL UpdateTrayTip(NOTIFYICONDATA notification, WORD messageID);
};

