// dllmain.cpp : 定义 DLL 应用程序的入口点。

#include "pch.h"
#include "resource.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "struct.h"
#include "UserInfo.h"

//socket
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include <thread>


//receive 
#include "receivemsg.h"


//只有windows下才可以这么写，别的平台需要增加配置项
#pragma comment(lib,"ws2_32.lib")


using namespace std;

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SentTextMessage(HWND hwndDlg);
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
WCHAR* CharToWChar(char* s);

void startSocket();

//定义变量
DWORD wxBaseAddress = 0;







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
		break;

	case WM_CLOSE:
		//关闭窗口事件
		EndDialog(hwndDlg, 0);
		break;
	case WM_COMMAND:

		//发送消息
		if (wParam == BTN_SEND_MSG)
		{
			// OutputDebugString(TEXT("发送消息按钮被点击"));
			//SentTextMessage(hwndDlg);
			wchar_t str[0x1000] = { 0,1,2 };
			SetDlgItemText(hwndDlg, TEXT_MY_INFO, str);


		}
		//接收消息
		if (wParam == BTN_RECE_MSG)
		{
			Information* myInfo = RecieveMsg();
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
			SetDlgItemText(hwndDlg, TEXT_RECEIVE_MSG, str);


		}
		//登录
		if (wParam == BTN_LOGIN)
		{
			OutputDebugString(TEXT("自己的微信ID按钮被点击："));
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

//将int转成16进制字符串
string Dec2Hex(DWORD i)
{
	//定义字符串流
	stringstream ioss;
	//存放转化后字符
	string s_temp;
	//以十六制(大写)形式输出
	ioss.fill('0');
	ioss << setiosflags(ios::uppercase) << setw(8) << hex << i;
	//以十六制(小写)形式输出//取消大写的设置
	//ioss << resetiosflags(ios::uppercase) << hex << i;
	ioss >> s_temp;
	return "0x" + s_temp;
}

//把string 转换为 LPCWSTR
LPCWSTR String2LPCWSTR(string text)
{
	//原型：
	//typedef _Null_terminated_ CONST WCHAR *LPCWSTR, *PCWSTR;
	//typedef wchar_t WCHAR;

	size_t size = text.length();
	WCHAR* buffer = new WCHAR[size + 1];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, buffer, size + 1);

	//确保以 '\0' 结尾
	buffer[size] = 0;
	return buffer;
}

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

VOID SentTextMessage(HWND hwndDlg)
{
	string text = "";

	DWORD callAddress_SendText = wxBaseAddress + 0xD0E62;


	text = "Call地址:";
	text.append(Dec2Hex(callAddress_SendText));
	//text.append("wxid_4sy2barbyny712");

	OutputDebugString(String2LPCWSTR(text));

	//组装wxid数据
	WCHAR wxid[50];
	//UINT uINT = GetDlgItemText(hwndDlg, IDC_WXID, wxid, 50);
	//if (uINT == 0)
	//{
	//	MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
	//	return;
	//}

	text = "目标wxid:\t";
	text.append(WcharToString(wxid));
	OutputDebugString(String2LPCWSTR(text));

	StructWxid structWxid = { 0 };
	structWxid.pWxid = wxid;
	structWxid.length = wcslen(wxid);
	structWxid.maxLength = wcslen(wxid) * 2;

	text = "微信ID长度:";
	text.append(Dec2Hex(structWxid.length));
	OutputDebugString(String2LPCWSTR(text));


	//structWxid.Init();
	//取wxid的地址
	DWORD* asmWxid = (DWORD*)&structWxid.pWxid;


	//组装发送的文本数据
	WCHAR wxMsg[1024];
	UINT uINT = GetDlgItemText(hwndDlg, TEXT_SEND_MSG, wxMsg, 1024);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写要发送的文本", "错误", MB_OK | MB_ICONERROR);
		return;
	}
	text = "发送内容:\t";
	text.append(WcharToString(wxMsg));
	OutputDebugString(String2LPCWSTR(text));

	StructWxid structMessage = { 0 };
	structMessage.pWxid = wxMsg;
	structMessage.length = wcslen(wxMsg);
	structMessage.maxLength = wcslen(wxMsg) * 2;

	text = "发送内容长度:";
	text.append(Dec2Hex(structMessage.length));
	OutputDebugString(String2LPCWSTR(text));

	//structMessage.Init();
	//取msg的地址
	DWORD* asmMsg = (DWORD*)&structMessage.pWxid;

	//定义一个缓冲区
	BYTE buff[0x81C] = { 0 };


}



void startSocket() {

	HWND   hwnd = NULL;

	char debugInfo[0x1000] = { 0 };

	//启动Windows socket2.X环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	//启动windows中socket网络环境
	WSAStartup(ver, &dat);

	// 编写网络通信代码

	//用socket API建立简易TCP服务端，需要6个步骤：
	//	1. 建立一个socket。 
	// IPV4，流，TCP的socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//	2. 绑定接受客户端连接的端口
	SOCKADDR_IN _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");


	if (SOCKET_ERROR == bind(_sock, (SOCKADDR*)&_sin, sizeof(_sin))) {
		// 错误处理
		sprintf_s(debugInfo, "[Error] => %s", "绑定网络端口失败...");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	else {
		MessageBox(NULL, L"绑定网络端口成功", L"错误", MB_OK);
		sprintf_s(debugInfo, "[Success] => %s", "绑定网络端口成功...");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
	};

	//	3. 监听网络端口。最大等待数是5.
	if (SOCKET_ERROR == listen(_sock, 5)) {
		MessageBox(NULL, L"监听网络端口失败", L"错误", MB_OK);
		sprintf_s(debugInfo, "[Error] => %s", "监听网络端口失败...");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
		return;
	}
	else
	{
		MessageBox(NULL, L"监听网络端口成功", L"错误", MB_OK);
		sprintf_s(debugInfo, "[Success] => %s", "监听网络端口成功...");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);

	};

	//	4. 等待接收客户端连接 
	SOCKADDR_IN clientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	SOCKET _cSock = INVALID_SOCKET;

	// 加入循环，多个客户端连接
	char msgBuf[] = "Hello i am server.";

	MessageBox(NULL, L"before 客户端连接", L"错误", MB_OK);
	while (true) {
		// 返回socket接收的长度
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock) {
			MessageBox(NULL, L"接受到无效客户端socket", L"错误", MB_OK);
			sprintf_s(debugInfo, "[Error] => %s", "接受到无效客户端socket...");
			SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);
			return;
		}
		MessageBox(NULL, L"新客户端加入", L"错误", MB_OK);
		//printf("新客户端加入：ip = %s \n", inet_ntoa(clientAddr.sin_addr));
		sprintf_s(debugInfo, "[Success] => %s", "新客户端加入");
		SetDlgItemTextA(hwnd, DEBUG_INFO, debugInfo);

		//	5. 向客户端发送一条数据 
		send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);

	}

	MessageBox(NULL, L"guanbi ", L"错误", MB_OK);
	//	6. 关闭socket 
	closesocket(_sock);
	WSACleanup();

}