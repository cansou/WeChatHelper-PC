#include "pch.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <time.h>
#include <sstream>
#include <iostream>
#include <iomanip>

//#include "resource.h"
#include "shellapi.h"
#include <tchar.h> 

using namespace std;

DWORD getWeChatWinAddr();
VOID setGlobalHwnd(HWND hwnd);
HWND getGlobalHwnd();
char* UnicodeToUtf8(const wchar_t* unicode);
wchar_t* UTF8ToUnicode(const char* str);
std::string EncodeConvert(std::string sInfo, INT sourceEncode, INT targetEncode);
wchar_t* StrToWchar(std::string str);
LPCWSTR stringToLPCWSTR(std::string orig);
std::string LPCWSTR2String(LPCWSTR lpcwszStr);
WCHAR* CharToWChar(char* s);

string Dec2Hex(DWORD i);
//LPCWSTR String2LPCWSTR(string text);
string WcharToString(WCHAR* wchar);

//读取内存中的字符串
LPCWSTR GetMsgByAddress(DWORD memAddress);
wstring String2Wstring(string str);

void DebugInfo(string text);