// WeChatHelperPC.cpp : 定义应用程序的入口点。
//
#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS 

//#include <Windows.h>
#include "pch.h"
#include "resource.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "StructInfo.h"
#include "UserInfo.h"
#include "receivemsg.h"
#include "SendMsg.h"
#include "MySqlTool.h"
#include "Utils.h"
#include <CommCtrl.h>

#include "EasyTcpServer.hpp"
#include<thread>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SocketServer();
void DebugInfo(string text);

//定义变量
bool g_bRun = true;
EasyTcpServer server;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//CELLLog::Instance().setLogPath("WeChatHelper.txt", "a");

		HANDLE hANDLE = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowDemoUI, hModule, NULL, 0);
		if (hANDLE != 0)
		{
			CloseHandle(hANDLE);
		}
		break;

	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//显示操作的窗口
VOID ShowDemoUI(HMODULE hModule)
{
	//DWORD wxBaseAddress = 0;
	//wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));

	DialogBox(hModule, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
}


//窗口回调函数，处理窗口事件
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{


	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		setGlobalHwnd(hwndDlg);
		SetDlgItemText(hwndDlg, DEBUG_INFO, L"dll注入成功，已开始监听微信数据。");

		// 接收消息
		HANDLE hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HookWx, NULL, NULL, 0);
		if (hookThread != 0) {
			CloseHandle(hookThread);
		}
		break;
	}

	case WM_CLOSE:
		//关闭窗口事件
		EndDialog(hwndDlg, 0);
		break;
	case WM_COMMAND:

		// 用户信息
		if (wParam == BTN_USERINFO)
		{
			if (isLogin() == 0)
			{
				SetDlgItemText(hwndDlg, TEXT_MY_INFO, L"请先登录微信");
			}
			else
			{
				Information* myInfo = GetMyInfo();
				wchar_t str[0x1000] = { 0 };
				swprintf_s(str,
					L"微信ID：%s\r\n账号：%s\r\n昵称：%s\r\n设备：%s\r\n手机号：%s\r\n邮箱：%s\r\n性别：%s\r\n国籍：%s\r\n省份：%s\r\n城市：%s\r\n签名：%s\r\n头像：%s",
					myInfo->wxid,
					myInfo->account,
					myInfo->nickname,
					myInfo->device,
					myInfo->phone,
					myInfo->email,
					myInfo->sex,
					myInfo->nation,
					myInfo->province,
					myInfo->city,
					myInfo->signName,
					myInfo->header);
				SetDlgItemText(hwndDlg, TEXT_MY_INFO, str);
			}
		
			break;

		}

		

		if (wParam == BTN_SOCKET)
		{
			//CELLLog::Instance().setLogPath("WeChatHelper.txt", "a");
			//CELLLog::Info("BTN_SOCKET_CLIENT begin.\n");

			DebugInfo("BTN_SOCKET begin...");

			thread t1(SocketServer);
			t1.detach();

			break;
		}

		
		if (wParam == BTN_SEND_MSG)
		{
			DebugInfo("BTN_SEND_MSG begin...");

			thread t1(SentTextMessage);
			t1.detach();

			break;
		}
		

	default:
		break;
	}
	return FALSE;
}


void SocketServer() {


	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(1);

	while (g_bRun)
	{
		server.OnRun();
		//printf("空闲时间处理其它业务..\n");
	}

	server.Close();
	CELLLog::Info("SocketServer exit");
}
