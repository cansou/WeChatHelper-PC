// WeChatHelperPC.cpp : 定义应用程序的入口点。
//
#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "WeChatHelperPC.h"
#include <sdkddkver.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <shlwapi.h>
#include <atlconv.h>
#include <io.h>
#include <string>

#include "EasyTcpServer.hpp"
#include<thread>

//只有windows下才可以这么写，别的平台需要增加配置项
#pragma comment(lib,"ws2_32.lib")

using namespace std;

const wchar_t* dllName = L"WeChatHelperDll.dll";

const wchar_t* weChatName = L"WeChat.exe";
HWND   hwnd;
bool g_bRun = true;

INT_PTR CALLBACK Dlgproc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
VOID startWechat();
BOOL CloseWeChat();
VOID InjectDll();
BOOL CheckInject(DWORD dwProcessid);
VOID UnInjectDll();
void SocketServer();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CELLLog::Instance().setLogPath("WeChatHelperPC.txt", "a");

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	HANDLE hObject = ::CreateMutex(NULL, FALSE, L"WeChatBot_Mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (hObject != 0)
		{
			CloseHandle(hObject);
		}
		TerminateProcess(GetCurrentProcess(), 0);
		return 0;
	}
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, &Dlgproc);
	return 0;
}


INT_PTR CALLBACK Dlgproc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (uMsg == WM_INITDIALOG) {
		hwnd = hwndDlg;
	}
	else if (uMsg == WM_COMMAND)
	{
		switch (wParam)
		{
		case BTN_INJECT:
			InjectDll();
			break;
		case BTN_UN_INJECT:
			UnInjectDll();
			break;
		case BTN_START_WECHAT:
			startWechat();
			break;
		case BTN_CLOSE_WECHAT:
			CloseWeChat();
			break;
		case BTN_SOCKET_SERVER:
		{
			CELLLog::Info("BTN_SOCKET_SERVER begin");
			thread t1(SocketServer);
			t1.detach();
			break;
		}
		case BTN_SOCKET_CLOSE:
		{
			g_bRun = false;
			break;
		}

		

		default:
			break;
		}
	}
	else if (uMsg == WM_CLOSE)
	{
		EndDialog(hwndDlg, 0);
		TerminateProcess(GetCurrentProcess(), 0);
	}
	return 0;
}

/**
 * 获取DLL文件路径
 * @param
 * @return string
 */
string GetDllPath()
{
	char DllPath[0x1000] = { 0 };
	sprintf_s(DllPath, "%s\\%ws", _getcwd(NULL, 0), dllName);
	if (_access(DllPath, 0) == -1)
	{
		return "";
	}
	return string(DllPath);

	
}

/**
 * 启动微信进程
 * @param
 * @return VOID
 */
VOID startWechat()
{
	char debugInfo[0x1000] = { 0 };
	string DllPath = GetDllPath();

	//DllPath =  "C:\\workspaces\\Flower\\WeChatHelper-PC\\WeChatHelperPC\\Debug\\WeChatHelperDll.dll";
	if (DllPath == "")
	{
		sprintf_s(debugInfo, "[Error] => %s => %s", DllPath.c_str(), "DLL不存在");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Tencent\\WeChat", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		sprintf_s(debugInfo, "[Error] => %s", "注册表打开失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	TCHAR szProductType[MAX_PATH] = {};
	DWORD dwBufLen = MAX_PATH;
	if (RegQueryValueEx(hKey, L"InstallPath", NULL, NULL, (LPBYTE)szProductType, &dwBufLen) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		sprintf_s(debugInfo, "[Error] => %s", "注册表查询失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}

	RegCloseKey(hKey);
	wchar_t weChatexe[MAX_PATH] = {};
	swprintf_s(weChatexe, L"%s\\%s", (wchar_t*)szProductType, weChatName);

	if (_waccess(weChatexe, 0) == -1)
	{
		sprintf_s(debugInfo, "[Error] => %ws => %s", weChatexe, "WeChat.exe不存在注册表安装目录");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	CreateProcess(NULL, weChatexe, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

	LPVOID Param = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (Param == 0)
	{
		sprintf_s(debugInfo, "[Error] => %s", "VirtualAllocEx失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	if (WriteProcessMemory(pi.hProcess, Param, DllPath.c_str(), MAX_PATH, NULL) == 0)
	{
		sprintf_s(debugInfo, "[Error] => %s", "DLL写入失败失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	HMODULE Kernel32 = GetModuleHandle(L"Kernel32.dll");
	if (Kernel32 == 0)
	{
		sprintf_s(debugInfo, "[Error] => %s", "Kernel32.dll获取失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	FARPROC address = GetProcAddress(Kernel32, "LoadLibraryA");
	HANDLE hRemote = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, Param, 0, NULL);
	ResumeThread(pi.hThread);

	sprintf_s(debugInfo, "[Success] => %s", "微信启动成功");
	SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
	return;
}

/**
 * 结束所有微信进程
 * @param
 * @return BOOL
 */
BOOL CloseWeChat()
{
	char debugInfo[0x1000] = { 0 };

	HANDLE ProcesssAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 proessInfo = { 0 };
	proessInfo.dwSize = sizeof(PROCESSENTRY32);
	do
	{
		if (wcscmp(weChatName, proessInfo.szExeFile) == 0) {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, proessInfo.th32ProcessID);
			if (hProcess != NULL) {
				TerminateProcess(hProcess, 0);
			}
		}
	} while (Process32Next(ProcesssAll, &proessInfo));
	sprintf_s(debugInfo, "[Success] => %s", "微信已全部关闭");
	SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
	return TRUE;
}

/**
 * 向已启动微信进程注入DLL
 * @param
 * @return VOID
 */
VOID InjectDll()
{
	char debugInfo[0x1000] = { 0 };

	string DllPath = GetDllPath();

	if (DllPath == "")
	{
		sprintf_s(debugInfo, "[Error] => %s => %s", DllPath.c_str(), "DLL不存在");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hProcess, &pe32) != TRUE)
	{
		sprintf_s(debugInfo, "[Error] => %s", "Process32First失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	HMODULE Kernel32 = GetModuleHandle(L"Kernel32.dll");
	if (Kernel32 == 0)
	{
		sprintf_s(debugInfo, "[Error] => %s", "Kernel32.dll获取失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	do
	{
		if (wcscmp(weChatName, pe32.szExeFile) != 0)
		{
			continue;
		}
		DWORD dwPid = pe32.th32ProcessID;
		//检测dll是否已经注入
		if (CheckInject(dwPid) != FALSE)
		{
			continue;
		}

		//2)	打开微信进程，获得HANDLE（OpenProcess）。
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (!hProcess)
		{
			continue;
		}
		//3)	在微信进程中为DLL文件路径字符串申请内存空间（VirtualAllocEx）。
		LPVOID pAddress = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!pAddress)
		{
			// 内存分配失败
			CloseHandle(hProcess);
			continue;
		}

		//4)	把DLL文件路径字符串写入到申请的内存中（WriteProcessMemory）
		DWORD dwWrite = 0;
		//if (WriteProcessMemory(hProcess, pAddress, DllPath.c_str(), MAX_PATH, &dwWrite) != 0)
		if (WriteProcessMemory(hProcess, pAddress, DllPath.c_str(), MAX_PATH, NULL) != 0)
		{

			//5)	从Kernel32.dll中获取LoadLibraryA的函数地址（GetModuleHandle、GetProcAddress）
			LPVOID pLoadLibraryAddress = GetProcAddress(Kernel32, "LoadLibraryA");

			//6)	在微信中启动内存中指定了文件名路径的DLL（CreateRemoteThread）。
			//也就是调用DLL中的DllMain（以DLL_PROCESS_ATTACH为参数）。
			HANDLE hRemote = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryAddress, pAddress, 0, NULL);
			if (hRemote != 0)
			{
				// 内存写入失败
				CloseHandle(hRemote);
			}
			ResumeThread(hProcess);
		}
		CloseHandle(hProcess);
	} while (Process32Next(hProcess, &pe32));

	sprintf_s(debugInfo, "[Success] => %s", "DLL注入成功");
	SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
}

/**
 * 检测微信是否已注入DLL
 * @param
 * @return BOOL
 */
BOOL CheckInject(DWORD dwProcessid)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	//初始化模块信息结构体
	MODULEENTRY32 ME32 = { sizeof(MODULEENTRY32) };
	//创建模块快照 1 快照类型 2 进程ID
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessid);
	//如果句柄无效就返回false
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//通过模块快照句柄获取第一个模块的信息
	if (!Module32First(hModuleSnap, &ME32))
	{
		//获取失败则关闭句柄
		CloseHandle(hModuleSnap);
		return FALSE;
	}
	do
	{
		if (wcscmp(ME32.szModule, dllName) == 0)
		{
			return TRUE;
		}

	} while (Module32Next(hModuleSnap, &ME32));
	return FALSE;
}

/**
 * 卸载微信已注入的DLL
 * @param
 * @return VOID
 */
VOID UnInjectDll()
{
	char debugInfo[0x1000] = { 0 };
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//1)	遍历系统中的进程，找到微信进程（CreateToolhelp32Snapshot、Process32Next）
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hProcess, &pe32) != TRUE)
	{
		sprintf_s(debugInfo, "[Error] => %s", "Process32First失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	HMODULE Kernel32 = GetModuleHandle(L"Kernel32.dll");
	if (Kernel32 == 0)
	{
		sprintf_s(debugInfo, "[Error] => %s", "Kernel32.dll获取失败");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	int wechatCount = 0;
	do
	{
		if (wcscmp(pe32.szExeFile, weChatName) != 0)
		{
			continue;
		}
		DWORD dwPid = pe32.th32ProcessID;

		//遍历模块
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);

		if (hSnap == INVALID_HANDLE_VALUE)
		{
			continue;
		}
		MODULEENTRY32 ME32 = { 0 };
		ME32.dwSize = sizeof(MODULEENTRY32);
		BOOL isNext = Module32First(hSnap, &ME32);
		BOOL flag = FALSE;
		while (isNext)
		{
			if (wcscmp(ME32.szModule, dllName) == 0)
			{
				flag = TRUE;
				break;
			}
			isNext = Module32Next(hSnap, &ME32);
		}
		if (flag != TRUE)
		{
			CloseHandle(hSnap);
			continue;
		}
		//打开目标进程
		HANDLE hPro = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (!hPro)
		{
			CloseHandle(hSnap);
			continue;
		}
		//获取FreeLibrary函数地址
		FARPROC pFun = GetProcAddress(Kernel32, "FreeLibrary");

		//创建远程线程执行FreeLibrary
		HANDLE hThread = CreateRemoteThread(hPro, NULL, 0, (LPTHREAD_START_ROUTINE)pFun, ME32.modBaseAddr, 0, NULL);
		if (hThread != NULL)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		CloseHandle(hPro);
		CloseHandle(hSnap);

	} while (Process32Next(hProcess, &pe32));

	sprintf_s(debugInfo, "[Success] => %s", "Dll卸载成功");
	SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
	return;
}



void SocketServer() {

	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);


	CELLLog::Info("before  g_bRun \n");
	while (g_bRun)
	{
		server.OnRun();
		//printf("空闲时间处理其它业务..\n");
	}

	server.Close();
	CELLLog::Info("SocketServer exit");
}