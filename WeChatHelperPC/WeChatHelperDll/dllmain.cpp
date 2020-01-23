// dllmain.cpp : 定义 DLL 应用程序的入口点。


#pragma comment(lib,"ws2_32.lib")

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
#include "MySqlTool.h"
#include "Utils.h"
#include <CommCtrl.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
// socket
#include "EasyTcpClient.hpp"
#include<thread>

using namespace std;



//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
WCHAR* CharToWChar(char* s);
void cmdThread();
void ConnSocket();


//定义变量
DWORD wxBaseAddress = 0;
bool g_bRun = true;
HWND   hwnd;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
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
	//获取WeChatWin.dll的基址
	wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	string text = "微信基址：\t";
	text.append(Dec2Hex(wxBaseAddress));
	OutputDebugString(String2LPCWSTR(text));

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


		cout << "hello world" << endl;

		while (true)
		{

			time_t mytime = time(0);
			wstring mydata = TEXT("\nC++ Time");
			mydata.append(to_wstring(mytime));
			OutputDebugString(mydata.c_str());
			Sleep(1000);

		}

		//// 登录状态
		//HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getLoginStatus, NULL, NULL, 0);
		//if (lThread != 0) {
		//	CloseHandle(lThread);
		//}

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
		}
		break;
		if (wParam == BTN_SOCKET)
		{
			//std::thread t1(ConnSocket);
			//t1.detach();

			setGlobalHwnd(hwndDlg);
			SetDlgItemText(hwndDlg, DEBUG_INFO, L"socket。");


		}
		break;
	default:
		break;
	}
	return FALSE;
}

WCHAR* CharToWChar(char* s)
{
	int w_nlen = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
	WCHAR* ret = (WCHAR*)malloc(sizeof(WCHAR) * w_nlen);
	memset(ret, 0, sizeof(ret));
	MultiByteToWideChar(CP_ACP, 0, s, -1, ret, w_nlen);
	return ret;
}

//文本消息结构体
struct StructWxid
{
	//发送的文本消息指针
	wchar_t* pWxid;
	//字符串长度
	DWORD length;
	//字符串最大长度
	DWORD maxLength;

	//补充两个占位数据
	DWORD fill1;
	DWORD fill2;
};

////将int转成16进制字符串
//string Dec2Hex(DWORD i)
//{
//	//定义字符串流
//	stringstream ioss;
//	//存放转化后字符
//	string s_temp;
//	//以十六制(大写)形式输出
//	ioss.fill('0');
//	ioss << setiosflags(ios::uppercase) << setw(8) << hex << i;
//	//以十六制(小写)形式输出//取消大写的设置
//	//ioss << resetiosflags(ios::uppercase) << hex << i;
//	ioss >> s_temp;
//	return "0x" + s_temp;
//}

////把string 转换为 LPCWSTR
//LPCWSTR String2LPCWSTR(string text)
//{
//	//原型：
//	//typedef _Null_terminated_ CONST WCHAR *LPCWSTR, *PCWSTR;
//	//typedef wchar_t WCHAR;
//
//	size_t size = text.length();
//	WCHAR* buffer = new WCHAR[size + 1];
//	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, buffer, size + 1);
//
//	//确保以 '\0' 结尾
//	buffer[size] = 0;
//	return buffer;
//}

string WcharToString(WCHAR* wchar)
{
	WCHAR* wText = wchar;
	// WideCharToMultiByte的运用
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	// psText为char*的临时数组，作为赋值给std::string的中间变量
	char* psText = new char[dwNum];
	// WideCharToMultiByte的再次运用
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	// std::string赋值
	return psText;
}



void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		//scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}


void ConnSocket() {

	char debugInfo[0x1000] = { 0 };
	sprintf_s(debugInfo, "[Success] => %s", "socket连接成功");
	SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);

	////const int cCount = FD_SETSIZE - 1;
	//const int cCount = 1;
	//EasyTcpClient* client[cCount];

	//for (int n = 0; n < cCount; n++)
	//{
	//	client[n] = new EasyTcpClient();
	//}
	//for (int n = 0; n < cCount; n++)
	//{
	//	client[n]->Connect("127.0.0.1", 4567);
	//}

	////启动UI线程
	//std::thread t1(cmdThread);
	//t1.detach();

	//Login login;
	//strcpy_s(login.userName, "lyd");
	//strcpy_s(login.PassWord, "lydmm");
	//while (g_bRun)
	//{
	//	for (int n = 0; n < cCount; n++)
	//	{
	//		client[n]->SendData(&login);
	//		client[n]->OnRun();
	//	}

	//	//printf("空闲时间处理其它业务..\n");
	//	//SetDlgItemText(hwndDlg, TEXT_MY_INFO, L"socket..");

	//	char debugInfo[0x1000] = { 0 };
	//	sprintf_s(debugInfo, "[Success] => %s", "socket连接成功");
	//	SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
	//	Sleep(1000);
	//}

	//for (int n = 0; n < cCount; n++)
	//{
	//	client[n]->Close();
	//}
}