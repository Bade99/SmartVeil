#pragma once

#include <windows.h>
#include <string>
#include <map>

#include "Manager.h"
#include "Settings.h"
#include "utils.cpp"

/// <summary>
/// Character separator for key value pairs in the "startup info" save file
/// </summary>
#define STARTUP_INFO_SEPARATOR TEXT('=')

//· None of its strings should have a slash at the end, you have to do + L"\\" later, extension should not include the dot "."
/// <summary>
/// Structure that contains all the parts to make the full path to the save info file
/// <para>None of its strings should have a slash at the end (you'll have to do + L"\\" later)</para>
/// <para>The extension should not include the dot "."</para>
/// </summary>
struct STARTUP_INFO_PATH {
	std::wstring known_folder; //eg. path to appdata
	std::wstring info_folder; //eg. smartveil
	std::wstring info_file; //eg. info
	std::wstring info_extension; //eg txt
};

/// <summary>
/// All the required information for application startup
/// </summary>
struct STARTUP_INFO {
#define SCV_FOREACH_STARTUP_INFO(STRUCT_MEMBER) \
	STRUCT_MEMBER(MANAGER,manager,{}) \
	STRUCT_MEMBER(SETTINGS,settings,{}) \
	
	SCV_FOREACH_STARTUP_INFO(SCV_GENERATE_STRUCT_MEMBER);
};


inline void ReadStartupInfoString(std::map<const std::wstring, std::wstring>stringed_struct, STARTUP_INFO* startup_info) {
	startup_info->settings.from_wstring_map(stringed_struct);
	startup_info->manager.from_wstring_map(stringed_struct,startup_info->settings);

	//Hack to check that every parameter is being initialized, since some members need information from others we cant do macro iteration
	//Update the number each time a new member is added or removed from STARTUP_INFO ! and initialize the new member
#define SCV_CHECK_STRUCT_MEMBER_COUNT(type,name,default_value) -1
	static_assert(!(2 SCV_FOREACH_STARTUP_INFO(SCV_CHECK_STRUCT_MEMBER_COUNT)), "Some member of STARTUP_INFO struct is not being initialized, you probably need to add another .from_wstring_map() and update the counter");
#undef  SCV_CHECK_STRUCT_MEMBER_COUNT
}

inline STARTUP_INFO read_startup_info_file(std::wstring file) {
	STARTUP_INFO startup_info;
	HANDLE info_file_handle = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE
		, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (info_file_handle != INVALID_HANDLE_VALUE) {
#define INFO_FILE_SIZE 5000 //TODO(fran): cleaner solution?
		WCHAR buf[INFO_FILE_SIZE];
		DWORD bytes_read;
		BOOL read_res = ReadFile(info_file_handle, buf, INFO_FILE_SIZE, &bytes_read, NULL);
#undef INFO_FILE_SIZE 

		if (read_res) {

			std::map<const std::wstring, std::wstring> info_mapped = mappify(buf, STARTUP_INFO_SEPARATOR);
			ReadStartupInfoString(info_mapped, &startup_info);
		}

		CloseHandle(info_file_handle);
	}
	return startup_info; //TODO(fran): move constructor
}

/// /// <summary>
/// Creates a string with the correct formatting to be saved and later retrieved
/// </summary>
/// <param name="startup_info"></param>
inline std::wstring WriteStartupInfoString(const STARTUP_INFO& startup_info) {
	std::wstring info_buf;
	//TODO(fran): ugly to have to put startup_info inside the macro
#define SCV_STRUCT_MEMBER_TO_WSTRING_MAP(type,name,default_value) startup_info.name.to_wstring_map(),
	std::map<const std::wstring, std::wstring> window_strings[] { SCV_FOREACH_STARTUP_INFO(SCV_STRUCT_MEMBER_TO_WSTRING_MAP)};
#undef  SCV_STRUCT_MEMBER_TO_WSTRING_MAP

	for (auto window : window_strings)
		for (auto pair : window)
			info_buf += pair.first + STARTUP_INFO_SEPARATOR + pair.second + L'\n';
	return info_buf;
}

/// <summary>
/// Saves the STARTUP_INFO struct to the desired path
/// </summary>
/// <param name="directory">Do not put slash at the end, eg C:\Users\User\AppData\SmartVeil</param>
/// <param name="filename">Should also contain extesion, eg info.txt</param>
inline void SaveStartupInfo(const STARTUP_INFO& startup_info, std::wstring directory, std::wstring filename) {
	std::wstring info_buf=WriteStartupInfoString(startup_info);

	//INFO: the folder MUST have been created previously, CreateFile doesnt do it
	BOOL dir_ret = CreateDirectoryW((LPWSTR)directory.c_str(), NULL);

	if (!dir_ret) {
		Assert(GetLastError() != ERROR_PATH_NOT_FOUND);
	}

	std::wstring full_file_path = directory + L'\\' + filename;
	HANDLE info_file_handle = CreateFileW(full_file_path.c_str(), GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE
		, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (info_file_handle != INVALID_HANDLE_VALUE) {
		DWORD bytes_written;
		BOOL write_res = WriteFile(info_file_handle, (LPWSTR)info_buf.c_str(), info_buf.size() * sizeof(wchar_t), &bytes_written, NULL);
		CloseHandle(info_file_handle);
	}
	else {

	}
	//TODO(fran): do we do something if couldnt create/write the file?
}
