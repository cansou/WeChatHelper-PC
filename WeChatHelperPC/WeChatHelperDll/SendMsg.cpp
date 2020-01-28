#include "pch.h"
#include "resource.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "Utils.h"

using namespace std;


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


VOID SentTextMessage()
{

	DWORD wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	string text = "";

	//定位发送消息的Call的位置
	DWORD callAddress_SendText = wxBaseAddress + 0xD0E62;

	//组装wxid数据
	WCHAR wxid[50] = L"wxid_4j4mqsuzdgie22";
	StructWxid structWxid = { 0 };
	structWxid.pWxid = wxid;
	structWxid.length = wcslen(wxid);
	structWxid.maxLength = wcslen(wxid) * 2;

	//取wxid的地址
	DWORD* asmWxid = (DWORD*)&structWxid.pWxid;

	//组装发送的文本数据
	WCHAR wxMsg[1024] = L"12345";
	//uINT = GetDlgItemText(hwndDlg, IDC_WXMSG, wxMsg, 1024);
	//if (uINT == 0)
	//{
	//	MessageBoxA(NULL, "请填写要发送的文本", "错误", MB_OK | MB_ICONERROR);
	//	return;
	//}

	StructWxid structMessage = { 0 };
	structMessage.pWxid = wxMsg;
	structMessage.length = wcslen(wxMsg);
	structMessage.maxLength = wcslen(wxMsg) * 2;

	//取msg的地址
	DWORD* asmMsg = (DWORD*)&structMessage.pWxid;

	//定义一个缓冲区
	BYTE buff[0x81C] = { 0 };

	//执行汇编调用
	__asm
	{
		//wxid
		//0F149BA8    8B55 CC         mov edx, dword ptr ss : [ebp - 0x34]
		//0F149BAB    8D43 14         lea eax, dword ptr ds : [ebx + 0x14]
		//0F149BAE    6A 01           push 0x1
		//0F149BB0    50              push eax
		//0F149BB1    53              push ebx
		//0F149BB2    8D8D E4F7FFFF   lea ecx, dword ptr ss : [ebp - 0x81C]
		//0F149BB8    E8 83B02100     call WeChatWi.0F364C40
		//0F149BBD    83C4 0C         add esp, 0xC


		mov edx, asmWxid

		//传递参数
		push 0x1

		mov eax, 0x0
		push eax

		//微信消息内容
		mov ebx, asmMsg
		push ebx

		lea ecx, buff

		//调用函数
		call callAddress_SendText

		//平衡堆栈
		add esp, 0xC
	}
}

