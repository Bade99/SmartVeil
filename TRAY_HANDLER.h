#pragma once
#include <windows.h>
#include <map>
#include <strsafe.h> //for stringcchcopy
#include <commctrl.h>
//#pragma comment(lib,"comctl32.lib")

//Needs Common Controls library to be loaded
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

	/// <summary>
	/// Creates a new tray icon and loads it into the tray
	/// <para>Your hwnd must have its hInstance parameter set</para>
	/// </summary>
	/// <param name="hwnd">The window whose procedure will receive messages from the tray icon</param>
	/// <param name="uID">Can be any unique number that you assign</param>
	/// <param name="iconID">The icon's resource ID, for use in MAKEINTRESOURCE(iconID)</param>
	/// <param name="uCallbackMessage">The message code that will be sent to the hwnd when some event happens on the tray object</param>
	/// <param name="messageID">The message to show on mouseover, for use in MAKEINTRESOURCE(messageID)</param>
	/// <returns>TRUE if created, FALSE if failed</returns>
	BOOL CreateTrayIcon(HWND hwnd,UINT uID, LONG iconID,UINT uCallbackMessage, WORD messageID);

	/// <summary>
	/// Destroys the tray object associated with that hwnd and uID
	/// </summary>
	/// <param name="hwnd">The window used in CreateTrayIcon</param>
	/// <param name="uID">The uID used in CreateTrayIcon</param>
	/// <returns>TRUE if the tray icon existed and was deleted, FALSE otherwise</returns>
	BOOL DestroyTrayIcon(HWND hwnd,UINT uID);

	/// <summary>
	/// Updates language of the text shown on mouseover
	/// <para>TODO(fran): integration with LANGUAGE_MANAGER</para>
	/// </summary>
	BOOL UpdateTrayTips();


private:
	TRAY_HANDLER();
	~TRAY_HANDLER();

	//·Each tray icon (notifyicondata) is mapped by hwnd+uid (into uint)
	std::map<std::pair<HWND, UINT>, std::pair<NOTIFYICONDATA,WORD>> TrayElements;
	
	BOOL UpdateTrayTip(NOTIFYICONDATA notification, WORD messageID);
};

