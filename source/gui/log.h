#pragma once
#include <Windows.h>
#define LOGNAME L"debugmenu_log.txt"


class eLog {
public:
	static wchar_t path[MAX_PATH];

	static void Initialize();
	static void Message(const char* function, const char* format, ...);
};