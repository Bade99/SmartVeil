#pragma once
#include <windows.h>
#include <map>
#include "MacroStandard.h"

//TODO(fran): dangerously short names for a macro

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
	
	//------HORRIBLE MACROS ALERT------// I'm open to cleaner solutions to implement reflection while they remain fast and simple

	/// <summary>
	/// Elements of the enum, you cant choose their values, just the name
	/// <para> Eg. ENUM_MEMBER(SPANISH)  \ means you'll add the member SPANISH to the enum</para>
	/// <para> ENUM_MEMBER(SPANISH = 5)  \ is NOT allowed</para>
	/// </summary>
	#define SCV_FOREACH_LANGUAGE(ENUM_MEMBER) \
					ENUM_MEMBER(ENGLISH)  \
					ENUM_MEMBER(SPANISH)  \
	
	/// /// <summary>
	/// All the languages supported by the application
	/// </summary>
	enum class LANGUAGE 
	{
		SCV_FOREACH_LANGUAGE(SCV_GENERATE_ENUM_MEMBER)
	};

	//TODO(fran): I think I might be able to just use the contents of this string in each place, and not carry the extra load from having it on run time
	/// <summary>
	/// All the languages supported by the application, in string form
	/// </summary>
	constexpr static const wchar_t* LANGUAGE_STRING[] = { //const is not required but I feel it's clearer
		SCV_FOREACH_LANGUAGE(SCV_GENERATE_STRING_FROM_ENUM_MEMBER)
	};

	static bool IsValidLanguage(LANGUAGE lang) {
	#define SCV_LANG_CASE_TRUE(MEMBER) case LANGUAGE::MEMBER:return true;

		switch (lang) {
			SCV_FOREACH_LANGUAGE(SCV_LANG_CASE_TRUE)
		}
		return false;

	#undef SCV_LANG_CASE_TRUE
	}

	//------HORRIBLE MACROS END------//

	constexpr static LANGUAGE GetDefaultLanguage() { return LANGUAGE::ENGLISH; }

	/// <summary>
	/// Retrieves the current instance of the Language Manager
	/// </summary>
	/// <returns></returns>
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
	//慈n failure returns (LANGID)-3 if failed to change the language
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

