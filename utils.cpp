#pragma once
#include <vector>
#include <windows.h>
#include <Shlobj.h>
#include <cwctype> // iswspace
#include <algorithm> // remove_if
#include <map>
#include <sstream>

#define Assert(assertion) if(!(assertion))*(int*)NULL=0

//inline UINT ModifierToVK(UINT modifier) {
//	UINT vk=0;
//	if (modifier & MOD_ALT) vk = VK_MENU; //WTF?
//	else if (modifier & MOD_CONTROL) vk = VK_CONTROL;
//	else if (modifier & MOD_SHIFT) vk = VK_SHIFT;
//	//MOD_WIN is reserved for the OS, MOD_NOREPEAT is a behavior change
//	return vk;
//}

inline std::vector<UINT> SeparateModifiersToVK(UINT modifiers) {
	std::vector<UINT> mods;
	if (modifiers & MOD_CONTROL) mods.push_back(VK_CONTROL);
	if (modifiers & MOD_ALT) mods.push_back(VK_MENU); //WTF?
	if (modifiers & MOD_SHIFT) mods.push_back(VK_SHIFT);
	//MOD_WIN is reserved for the OS, MOD_NOREPEAT is a behavior change
	return mods;
}

inline BYTE HotkeyModifiersToHotkeyControlModifiers(WORD modifiers) {
	BYTE hcm = 0;
	if (modifiers & MOD_ALT) hcm |= HOTKEYF_ALT;
	if (modifiers & MOD_CONTROL) hcm |= HOTKEYF_CONTROL;
	if (modifiers & MOD_SHIFT) hcm |= HOTKEYF_SHIFT;
	return hcm;
}

inline WORD HotkeyControlModifiersToHotkeyModifiers(BYTE modifiers) {
	WORD hm = 0;
	if (modifiers & HOTKEYF_ALT) hm |= MOD_ALT;
	if (modifiers & HOTKEYF_CONTROL) hm |= MOD_CONTROL;
	if (modifiers & HOTKEYF_SHIFT) hm |= MOD_SHIFT;
	return hm;
}

//For MOD_CONTROL and the sort
inline std::wstring HotkeyString(WORD modifiers, BYTE vk) {
	std::wstring hotkey_str = L"";
	BOOL first_mod = TRUE;
	if (modifiers & MOD_CONTROL) {
		if (!first_mod) hotkey_str += L"+";
		hotkey_str += L"Ctrl";
		first_mod = FALSE;
	}
	if (modifiers & MOD_ALT) {
		if (!first_mod) hotkey_str += L"+";
		hotkey_str += L"Alt";
		first_mod = FALSE;
	}
	if (modifiers & MOD_SHIFT) {
		if (!first_mod) hotkey_str += L"+";
		hotkey_str += L"Shift";
		first_mod = FALSE;
	}
	if (vk) {
		LONG scancode = ((LONG)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC) << 16);
		WCHAR vk_str[20];
		GetKeyNameTextW(scancode, vk_str, 20);
		if (*vk_str) {
			if (!first_mod) hotkey_str += L"+";
			hotkey_str += vk_str;
			first_mod = FALSE;
		}
	}
	return hotkey_str;
}

//For HOTKEYF_ALT and the sort
inline std::wstring ControlHotkeyString(BYTE modifiers, BYTE vk) {
	WORD mods = HotkeyControlModifiersToHotkeyModifiers(modifiers);
	return HotkeyString(mods, vk);
}

inline BOOL isChecked(HWND checkbox) {
	return ((SendMessageW(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE);
}

inline void RemoveWhiteSpace(std::wstring &text) {
	text.erase(remove_if(text.begin(), text.end(), std::iswspace), text.end());
}

inline std::map<std::wstring, std::wstring> mappify(std::wstring const& s, wchar_t const& separator)
{
	std::map<std::wstring, std::wstring> m;

	std::wstring key, val;
	std::wistringstream iss(s);

	while (std::getline(std::getline(iss, key, separator) >> std::ws, val)) {
		RemoveWhiteSpace(key);
		RemoveWhiteSpace(val);//TODO(fran): it seems like the string to int/other functions already check for whitespace, do we take this one out?
		m[key] = val;
	}

	return m;
}

#define RECTWIDTH(r) (r.right >= r.left ? r.right - r.left : r.left - r.right )

#define RECTHEIGHT(r) (r.bottom >= r.top ? r.bottom - r.top : r.top - r.bottom )