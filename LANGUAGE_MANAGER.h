#pragma once
#include <windows.h>
#include <map>

//Request string
#define RS(stringID) LANGUAGE_MANAGER::Instance().RequestString(stringID)

//Request c-string
#define RCS(stringID) LANGUAGE_MANAGER::Instance().RequestString(stringID).c_str()

//Add window string
#define AWT(hwnd,stringID) LANGUAGE_MANAGER::Instance().AddWindowText(hwnd, stringID)

//Add combobox string in specific ID (Nth element of the list)
#define ACT(hwnd,ID,stringID) LANGUAGE_MANAGER::Instance().AddComboboxText(hwnd, ID, stringID);

class LANGUAGE_MANAGER
{
public://TODO(fran): add lang to the rest of the classes: outmgr,duplmgr,...
	
	enum LANGUAGE
	{
		ENGLISH = 0, SPANISH
	};

	static BOOL IsValidLanguage(int lang) {//TODO(fran): simpler way to check for valid enum without having to add each lang here
		return lang == LANGUAGE::ENGLISH || lang == LANGUAGE::SPANISH;
	}

	static LANGUAGE_MANAGER& Instance()
	{
		static LANGUAGE_MANAGER instance;

		return instance;
	}
	LANGUAGE_MANAGER(LANGUAGE_MANAGER const&) = delete;
	void operator=(LANGUAGE_MANAGER const&) = delete;

	//INFO: all Add... functions send the text update message when called, for dynamic hwnds you should create everything first and only then add it

	//嫂dds the hwnd to the list of managed hwnds and sets its text corresponding to stringID and the current language
	//愚any hwnds can use the same stringID
	//意ext time there is a language change the window will be automatically updated
	//愛eturns FALSE if invalid hwnd or stringID //TODO(fran): do the stringID check
	BOOL AddWindowText(HWND hwnd, UINT stringID);

	//愈pdates all managed objects to the new language, all the ones added after this call will also use the new language
	//慈n success returns the new LANGID (language) //TODO(fran): should I return the previous langid? it feels more useful
	//慈n failure returns (LANGID)-2 if the language is invalid, (LANGID)-3 if failed to change the language
	LANGID ChangeLanguage(LANGUAGE newLang);

	//愛eturns the requested string in the current language
	//弒f stringID is invalid returns L"" //TODO(fran): check this is true
	//INFO: uses temporary string that lives till the end of the full expression it appears in
	std::wstring RequestString(UINT stringID);

	//嫂dds the hwnd to the list of managed comboboxes and sets its text for the specified ID(element in the list) corresponding to stringID and the current language
	//意ext time there is a language change the window will be automatically updated
	BOOL AddComboboxText(HWND hwnd, UINT ID, UINT stringID);

	//Set the hinstance from where the string resources will be retrieved
	HINSTANCE SetHInstance(HINSTANCE hInst);

	//Add a control that manages other windows' text where the string changes
	//Each time there is a language change we will send a message with the specified code so the window can update its control's text
	//One hwnd can only be linked to one messageID
	BOOL AddDynamicText(HWND hwnd,UINT messageID);

private:
	LANGUAGE_MANAGER();
	~LANGUAGE_MANAGER();

	HINSTANCE hInstance=NULL;//TODO(fran): should we ask for the instance to each control? for now we dont really need it

	LANGUAGE CurrentLanguage=(LANGUAGE)-1;
	std::map<HWND, UINT> Hwnds;
	std::map<HWND,UINT> DynamicHwnds;
	std::map<std::pair<HWND, UINT>, UINT> Comboboxes;
	//Add list of hwnd that have dynamic text, therefore need to know when there was a lang change to update their text

	inline BOOL UpdateHwnd(HWND hwnd, UINT stringID);
	inline BOOL UpdateDynamicHwnd(HWND hwnd, UINT messageID);
	inline BOOL UpdateCombo(HWND hwnd, UINT ID, UINT stringID);

	LCID GetLCID(LANGUAGE lang);

	LANGID GetLANGID(LANGUAGE lang);
};

