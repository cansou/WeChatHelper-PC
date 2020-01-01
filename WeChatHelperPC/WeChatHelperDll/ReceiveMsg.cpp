#include "pch.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <CommCtrl.h>
#include "StructInfo.h"
#include "UserInfo.h"
#include "Utils.h"
#include "Offset.h"
#include "resource.h"
#include "MySqlTool.h"
#include <thread>

using namespace std;

DWORD recieveMsgParam, recieveMsgJmpAddr;




/**
 * 处理拦截到的消息内容
 */
void recieveMessageJump(DWORD esp)
{
	DWORD** msgAddress = (DWORD**)(esp + 0x20);
	recieveMsgStruct* msg = new recieveMsgStruct;
	msg->type = (int)(*((DWORD*)(**msgAddress + 0x30)));
	msg->isSelf = (int)(*((DWORD*)(**msgAddress + 0x34)));
	msg->fromWxid = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x40)));
	msg->content = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x68)));
	msg->senderWxid = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x114)));
	msg->unkonwStr = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x128)));


	////开个线程数据保持到mysql
	//thread t1(InsertData);
	//t1.detach();


	HWND listHwnd = GetDlgItem(getGlobalHwnd(), LIST_RECIEVE_MSG);
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;

	// 来源
	wchar_t type[0x100] = { 0 };
	swprintf_s(type, L"%d", msg->type);
	item.iSubItem = 0;
	item.pszText = type;
	ListView_InsertItem(listHwnd, &item);

	// self
	wchar_t isSelf[0x100] = { 0 };
	swprintf_s(isSelf, L"%d", msg->isSelf);
	item.iSubItem = 1;
	item.pszText = isSelf;
	ListView_SetItem(listHwnd, &item);

	// 来源
	item.iSubItem = 2;
	item.pszText = msg->fromWxid;
	ListView_SetItem(listHwnd, &item);

	// 发送者
	item.iSubItem = 3;
	item.pszText = msg->senderWxid;
	ListView_SetItem(listHwnd, &item);

	// 字符串
	item.iSubItem = 4;
	item.pszText = msg->unkonwStr;
	ListView_SetItem(listHwnd, &item);

	// 消息内容
	item.iSubItem = 5;
	item.pszText = msg->content;
	ListView_SetItem(listHwnd, &item);


}

/**
 * 被写入到hook点的接收消息裸函数
 */
__declspec(naked) void recieveMsgDeclspec()
{
	__asm
	{
		mov ecx, recieveMsgParam
		pushad
		push esp
		call recieveMessageJump
		add esp, 4
		popad
		jmp recieveMsgJmpAddr
	}
}


void RecieveMsgHook()
{
	//获取模块地址，直到获取为止
	while (true)
	{
		if (getWeChatWinAddr() != 0)
		{
			break;
		}
		Sleep(300);
	}

	//HOOK接收消息
	DWORD recieveMsgHookAddr = getWeChatWinAddr() + RECEIVE_MSG_HOOKADDR;
	recieveMsgParam = getWeChatWinAddr() + RECEIVEMSGHOOKPARM;
	recieveMsgJmpAddr = recieveMsgHookAddr + 5;
	BYTE msgJmpCode[5] = { 0xE9 };
	*(DWORD*)&msgJmpCode[1] = (DWORD)recieveMsgDeclspec - recieveMsgHookAddr - 5;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recieveMsgHookAddr, msgJmpCode, 5, NULL);
	SetDlgItemText(getGlobalHwnd(), FLAG_HOOK_MSG, L"消息拦截已开启");

}
