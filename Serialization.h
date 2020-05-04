#pragma once
#include <string>
#include <Windows.h>
#include "LANGUAGE_MANAGER.h"
#include "Settings.h"

//--------------------------------------------------------
//Defines serialization and deserialization for every type (preferably ones not expected to change, for complex structs go to MacroStandard.h "Reflection for serialization")
//--------------------------------------------------------

//-Assumes that the encoded variable is in a std::wstring all by itself, no whitespaces
//  Eg. an int would be "14", a float "3.555f", ...
//-Each serialize-deserialize function pair can decide the string format of the variable, eg SIZE might be encoded "{14,15}"

//-If the string does not contain the proper encoding for the variable it's just ignored, and nothing is modified, 
//  therefore everything is expected to be preinitialized

//int
inline std::wstring serialize(int v) { return std::to_wstring(v); }
inline void deserialize(int& v, const std::wstring& s) { try { v = stoi(s); } catch (...) {} }

//float
inline std::wstring serialize(float v) { return std::to_wstring(v); }
inline void deserialize(float& v, const std::wstring& s) { try { v = stof(s); } catch (...) {} }

//bool
inline std::wstring serialize(bool v) { return std::to_wstring(v); }
inline void deserialize(bool& v, const std::wstring& s) { try { v = stoi(s); } catch (...) {} }

//LANGUAGE_MANAGER::LANGUAGE
inline std::wstring serialize(LANGUAGE_MANAGER::LANGUAGE v) { 
	if (LANGUAGE_MANAGER::Instance().IsValidLanguage(v)) return LANGUAGE_MANAGER::LANGUAGE_STRING[(int)v]; 
	else return LANGUAGE_MANAGER::LANGUAGE_STRING[(int)LANGUAGE_MANAGER::GetDefaultLanguage()];
}
inline void deserialize(LANGUAGE_MANAGER::LANGUAGE& v, const std::wstring& s) {
	for (int i = 0; i < ARRAYSIZE(LANGUAGE_MANAGER::LANGUAGE_STRING); i++)
		if (!wcscmp(s.c_str(), *(LANGUAGE_MANAGER::LANGUAGE_STRING + i)))
		{ v = (LANGUAGE_MANAGER::LANGUAGE)i; return; }
}

//SIZE
inline std::wstring serialize(SIZE v) {
	using namespace std::string_literals;
	return L"{"s + std::to_wstring(v.cx) + L","s + std::to_wstring(v.cy) + L"}"s;
}
inline void deserialize(SIZE& v, const std::wstring& s) { //TODO(fran): use regex?
	size_t open = 0, comma = 0, close = 0;
	open = s.find(L'{', open);
	comma = s.find(L',', open + 1);
	close = s.find(L'}', comma + 1);
	if (open == std::wstring::npos || comma == std::wstring::npos || close == std::wstring::npos)
		return;
	std::wstring cx = s.substr(open + 1, comma - open);
	std::wstring cy = s.substr(comma + 1, close - comma);
	SIZE temp;
	try 
	{ 
		temp.cx = stol(cx);
		temp.cy = stol(cy);
		v = temp;
	}
	catch (...) {}
	//Allows anything that contains {number,number} somewhere in its string, eg: }}}}}{51,12}}{}{12,}{ is valid
}

//POINT
inline std::wstring serialize(POINT v) {
	using namespace std::string_literals;
	return L"{"s + std::to_wstring(v.x) + L","s + std::to_wstring(v.y) + L"}"s;
}
inline void deserialize(POINT& v, const std::wstring& s) { //TODO(fran): use regex?
	size_t open = 0, comma = 0, close = 0;
	open = s.find(L'{', open);
	comma = s.find(L',', open + 1);
	close = s.find(L'}', comma + 1);
	if (open == std::wstring::npos || comma == std::wstring::npos || close == std::wstring::npos)
		return;
	std::wstring x = s.substr(open + 1, comma - open);
	std::wstring y = s.substr(comma + 1, close - comma);
	POINT temp;
	try
	{
		temp.x = stol(x);
		temp.y = stol(y);
		v = temp;
	}
	catch (...) {}
	//Allows anything that contains {number,number} somewhere in its string, eg: }}}}}{51,12}}{}{12,}{ is valid
}


////HOTKEY (should probably choose a longer name SCV_HOTKEY or SETTINGS_HOTKEY)
//inline std::wstring serialize(HOTKEY v) {
//	using namespace std::string_literals;
//	return L"{"s + std::to_wstring(v.mods) + L","s + std::to_wstring(v.vk) + L"}"s;
//}
//inline void deserialize(HOTKEY& v, const std::wstring& s) { //TODO(fran): use regex?
//	size_t open = 0, comma = 0, close = 0;
//	open = s.find(L'{', open);
//	comma = s.find(L',', open + 1);
//	close = s.find(L'}', comma + 1);
//	if (open == std::wstring::npos || comma == std::wstring::npos || close == std::wstring::npos)
//		return;
//	std::wstring mods = s.substr(open + 1, comma - open);
//	std::wstring vk = s.substr(comma + 1, close - comma);
//	HOTKEY temp;
//	try
//	{
//		temp.mods = stoul(mods);
//		temp.vk = stoul(vk);
//		v = temp;
//	}
//	catch (...) {}
//	//Allows anything that contains {number,number} somewhere in its string, eg: }}}}}{51,12}}{}{12,}{ is valid
//}

