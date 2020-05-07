#pragma once

//-------------------------------------------------------
//Definition of the standard macros that the program uses
//-------------------------------------------------------

//----------------------------
//Reflection for serialization
//----------------------------

//STRUCT: struct variables should always be default initialized, so we provide three slots, one for the type, other for the name, and the last for the value
//----------------------------

//HACK to be able to initialize parameters with initializer lists
//Thanks to https://stackoverflow.com/questions/13842468/comma-in-c-c-macro/13842784
#define SCV_COMMA ,

//Initialization "function" to add a variable to a reflected struct
#define SCV_GENERATE_STRUCT_MEMBER(type,name,default_value) type name=default_value;

//Adds a string to an array of strings, to store the names of the variables of a reflected struct
#define SCV_GENERATE_STRING_FROM_STRUCT_MEMBER(type,name,default_value) L#name,

//Serialize a reflected struct, REQUIRES serialize to be defined: inline std::wstring serialize([SOMETYPE] v) {...}
#define SCV_SERIALIZE_STRUCT_MEMBER(type,name,default_value) serialize(name);

//Deserialize a reflected struct, REQUIRES deserialized to be defined: inline void deserialize([SOMETYPE]& v, const std::wstring& s){...} 
//and also std::wstring potential_string_value for you to assign the strings with the encoded variable values, go to Settings.h for more info
#define SCV_DESERIALIZE_STRUCT_MEMBER(type,name,default_value) deserialize(name,potential_string_value);
//TODO(fran): this is awful, something better is needed so we dont just leave potential_string_value for the caller to declare

//Example of a reflected struct:
//#define SCV_FOREACH_SETTINGS_MEMBER(STRUCT_MEMBER) \
//		STRUCT_MEMBER(HOTKEY, hotkey, { VK_F9 SCV_COMMA MOD_CONTROL }) \
//      STRUCT_MEMBER(LANGUAGE_MANAGER::LANGUAGE, language, LANGUAGE_MANAGER::LANGUAGE::ENGLISH)		\
//		STRUCT_MEMBER(VEIL_ON_STARTUP, show_veil_on_startup, VEIL_ON_STARTUP::YES) \
//		STRUCT_MEMBER(BOOL, start_with_windows, FALSE)		\
//Note: do NOT put spaces after the \, if you get weird errors make sure there are no spaces

//struct SETTINGS { SCV_FOREACH_SETTINGS_MEMBER(SCV_GENERATE_STRUCT_MEMBER) }; //We generate the struct by iterating over the members

//Now we can finally implement some reflection:
//	constexpr static const wchar_t* SETTINGS_STRING[] = { //Variable names
//	SCV_FOREACH_SETTINGS_MEMBER(SCV_GENERATE_STRING_FROM_STRUCT_MEMBER)
//	};
//You can iterate over SCV_FOREACH_SETTINGS_MEMBER to implement multiple other routines, for example, automatic serialization and deserialization
//You also know the number of variables of the struct SETTINGS by simply ARRAYSIZE(SETTINGS_STRING), or other methods

//ENUM: enums should either define all their values or none, we are going with none in this case, therefore we provide only one slot for the name
//----------------------------

//Initialization "function" to add a member to a reflected enum
#define SCV_GENERATE_ENUM_MEMBER(MEMBER) MEMBER,

//Adds a string to an array of strings, to store the names of the members of a reflected enum
#define SCV_GENERATE_STRING_FROM_ENUM_MEMBER(MEMBER) L#MEMBER,

//Usage is similar to STRUCT, go to LANGUAGE_MANAGER.h for more info, and other methods that are allowed thanks to reflection

//---------------------------------
//Starting index of window messages
//---------------------------------

//Refer to Manager.h for information on the messages sent and received by this window procedure
#define SCV_MANAGER_FIRST_MESSAGE (WM_USER+0)
#define SCV_MANAGER_FIRST_INTERNAL_MESSAGE (WM_USER+200)

//Refer to Veil.h for information on the messages sent and received by this window procedure
#define SCV_VEIL_FIRST_MESSAGE (WM_USER+500)
#define SCV_VEIL_FIRST_INTERNAL_MESSAGE (WM_USER+700)

//Refer to Settings.h for information on the messages sent and received by this window procedure
#define SCV_SETTINGS_FIRST_MESSAGE (WM_USER+1000)
#define SCV_SETTINGS_FIRST_INTERNAL_MESSAGE (WM_USER+1200)

///First index from which windows messages will be sent
#define SCV_CUSTOMFRAME_FIRST_MESSAGE (WM_USER+1500)
//Messages that will come from custom frames
#define SCV_CUSTOMFRAME_CLOSE (SCV_CUSTOMFRAME_FIRST_MESSAGE+1)
#define SCV_CUSTOMFRAME_MINIMIZE (SCV_CUSTOMFRAME_FIRST_MESSAGE+2)

//-----
//Defer (usage: defer{block of code;};)
//-----
//Thanks to https://handmade.network/forums/t/1273-post_your_c_c++_macro_tricks/3

template <typename F>
struct Defer {
	Defer(F f) : f(f) {}
	~Defer() { f(); }
	F f;
};

template <typename F>
Defer<F> makeDefer(F f) {
	return Defer<F>(f);
};

#define __defer( line ) defer_ ## line
#define _defer( line ) __defer( line )

struct defer_dummy { };
template<typename F>
Defer<F> operator+(defer_dummy, F&& f)
{
	return makeDefer<F>(std::forward<F>(f));
}

#define defer auto _defer( __LINE__ ) = defer_dummy( ) + [ & ]( )