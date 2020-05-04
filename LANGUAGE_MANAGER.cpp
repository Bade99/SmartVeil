#include "LANGUAGE_MANAGER.h"


LANGUAGE_MANAGER::LANGUAGE_MANAGER()
{
}


LANGUAGE_MANAGER::~LANGUAGE_MANAGER()
{
}

HINSTANCE LANGUAGE_MANAGER::SetHInstance(HINSTANCE hInst)
{
	HINSTANCE oldHInst = this->hInstance;
	this->hInstance = hInst; 
	return oldHInst;
}

BOOL LANGUAGE_MANAGER::AddDynamicText(HWND hwnd, UINT messageID)
{
	if (!hwnd) return FALSE;
	this->DynamicHwnds[hwnd] = messageID;
	this->UpdateDynamicHwnd(hwnd, messageID);
	return TRUE;
}

BOOL LANGUAGE_MANAGER::AddWindowText(HWND hwnd, UINT stringID)
{
	BOOL res = UpdateHwnd(hwnd,stringID);
	if (res) this->Hwnds[hwnd] = stringID;
	return res;
}

BOOL LANGUAGE_MANAGER::AddComboboxText(HWND hwnd, UINT ID, UINT stringID)
{
	BOOL res = UpdateCombo(hwnd, ID, stringID);
	if (res) this->Comboboxes[std::make_pair(hwnd,ID)] = stringID;
	return res;
}

LANGID LANGUAGE_MANAGER::ChangeLanguage(LANGUAGE newLang)
{
	//if (newLang == this->CurrentLanguage) return -1;//TODO: negative values wrap around to huge values, I assume not all lcid values are valid, find out if these arent

	this->CurrentLanguage = newLang;
	//LCID previousLang = SetThreadLocale(this->GetLCID(newLang));
	//INFO: thanks https://www.curlybrace.com/words/2008/06/10/setthreadlocale-and-setthreaduilanguage-for-localization-on-windows-xp-and-vista/
	// SetThreadLocale has no effect on Vista and above
	LANGID newLANGID = this->GetLANGID(newLang);
	//INFO: On non-Vista platforms, SetThreadLocale can be used. Instead of a language identifier, it accepts a locale identifier
	LANGID lang_res = SetThreadUILanguage(newLANGID); //If successful returns the same value that you sent it

	for (auto const& hwnd_sid : this->Hwnds)
		this->UpdateHwnd(hwnd_sid.first, hwnd_sid.second);

	for (auto const& hwnd_id_sid : this->Comboboxes)
		this->UpdateCombo(hwnd_id_sid.first.first, hwnd_id_sid.first.second, hwnd_id_sid.second);

	for (auto const& hwnd_msg : this->DynamicHwnds) {
		this->UpdateDynamicHwnd(hwnd_msg.first, hwnd_msg.second);
	}

	return (lang_res == newLANGID ? lang_res :-3);//is this NULL when failed?
}

std::wstring LANGUAGE_MANAGER::RequestString(UINT stringID)
{
	WCHAR text[500];
	LoadStringW(this->hInstance, stringID, text, 500);//INFO: last param is size of buffer in characters
	return text;
}

inline BOOL LANGUAGE_MANAGER::UpdateHwnd(HWND hwnd, UINT stringID)
{
	BOOL res = SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)this->RequestString(stringID).c_str()) == TRUE;
	InvalidateRect(hwnd, NULL, TRUE);
	return res;
}

inline BOOL LANGUAGE_MANAGER::UpdateDynamicHwnd(HWND hwnd, UINT messageID)
{
	return PostMessage(hwnd, messageID, 0, 0);
}

inline BOOL LANGUAGE_MANAGER::UpdateCombo(HWND hwnd, UINT ID, UINT stringID)
{
	UINT currentSelection = SendMessage(hwnd, CB_GETCURSEL, 0, 0);//TODO(fran): is there truly no better solution than having to do all this just to change a string?
	SendMessage(hwnd, CB_DELETESTRING, ID, 0);
	LRESULT res = SendMessage(hwnd, CB_INSERTSTRING, ID, (LPARAM)this->RequestString(stringID).c_str());
	if(currentSelection!= CB_ERR)SendMessage(hwnd, CB_SETCURSEL, currentSelection, 0);
	return res!= CB_ERR && res!= CB_ERRSPACE;//TODO(fran): can I check for >=0 with lresult?
}

LCID LANGUAGE_MANAGER::GetLCID(LANGUAGE lang)
{
	//TODO(fran): do we ask for this values on enum construction?
#ifdef _DEBUG
	static_assert(2 == ARRAYSIZE(LANGUAGE_STRING), "New languages not mapped to an LCID, add it and update the number after the assert");
#endif
	switch (lang) {
	case LANGUAGE::English:
		return MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	case LANGUAGE::Español:
		return MAKELCID(MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH), SORT_DEFAULT);
	default:
		return NULL;
	}
}

LANGID LANGUAGE_MANAGER::GetLANGID(LANGUAGE lang)
{
#ifdef _DEBUG
#define SCV_CHECK_ENUM_MEMBER_COUNT(member) -1
	static_assert(2 == ARRAYSIZE(LANGUAGE_STRING), "New languages not mapped to a LANGID, add it and update the number after the assert");
#undef  SCV_CHECK_ENUM_MEMBER_COUNT
#endif
	//INFO: https://docs.microsoft.com/en-us/windows/win32/intl/language-identifier-constants-and-strings
	switch (lang) {
	case LANGUAGE::English:
		return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	case LANGUAGE::Español:
		return MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH);
	default:
		return NULL;
	}
}
