
#include "pch.h"
#include "resource.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <strstream>
#include "Utils.h"
#include "MySqlTool.h"
#include <thread>

#pragma comment(lib, "Version.lib")

using namespace std;


VOID HookWx();
VOID RecieveMsg();
VOID RecieveMsgHook();
string Dec2Hex(DWORD i);
LPCWSTR GetMsgByAddress(DWORD memAddress);
wstring String2Wstring(string str);


//定义变量
//DWORD wxBaseAddress = 0;

//对话框句柄
HWND hWinDlg;
//跳回地址
DWORD jumBackAddress = 0;
//我们要提取的寄存器内容
DWORD r_esp = 0;
//我自己的微信ID
string myWxId = "";

CHAR originalCode[5] = { 0 };

//Hook接收消息
VOID HookWx()
{

		//WeChatWin.dll+0x310573
		int hookAddress = getWeChatWinAddr() + 0x325373;
		string debugMsg = "Hook address：\t";
		debugMsg.append(Dec2Hex(hookAddress));
		OutputDebugString(stringToLPCWSTR(debugMsg));

		//跳回的地址
		jumBackAddress = hookAddress + 5;

		//组装跳转数据
		BYTE jmpCode[5] = { 0 };
		jmpCode[0] = 0xE9;

		//新跳转指令中的数据=跳转的地址-原地址（HOOK的地址）-跳转指令的长度
		*(DWORD*)&jmpCode[1] = (DWORD)RecieveMsgHook - hookAddress - 5;

		//保存当前位置的指令,在unhook的时候使用。
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, originalCode, 5, 0);

		//覆盖指令 B9 E8CF895C //mov ecx,0x5C89CFE8
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);

}


//跳转到这里，让我们自己处理消息
__declspec(naked) VOID RecieveMsgHook()
{

		//保存现场
	__asm
	{

		mov ecx, 0x107871D8


		//提取esp寄存器内容，放在一个变量中
		mov r_esp, esp

		//保存寄存器
		pushad
		pushf
	}

	//调用接收消息的函数
	RecieveMsg();

	//恢复现场
	__asm
	{
		popf
		popad

		//跳回被HOOK指令的下一条指令
		jmp jumBackAddress
	}
}

VOID RecieveMsg()
{
	// 来源、发送者、内容存到数据库，只保持文本
	string fromWxid;
	string senderWxid;
	string content ;

	wstring receivedMessage = TEXT("");
	BOOL isFriendMsg = FALSE;
	//[[esp]]
	//信息块位置
	DWORD** msgAddress = (DWORD**)r_esp;

	//消息类型[[esp]]+0x30
	//[01文字] [03图片] [31转账XML信息] [22语音消息] [02B视频信息]
	DWORD msgType = *((DWORD*)(**msgAddress + 0x30));
	receivedMessage.append(TEXT("消息类型:"));
	switch (msgType)
	{
	case 0x01:
		receivedMessage.append(TEXT("文字"));
		break;
	case 0x03:
		receivedMessage.append(TEXT("图片"));
		break;

	case 0x22:
		receivedMessage.append(TEXT("语音"));
		break;
	case 0x25:
		receivedMessage.append(TEXT("好友确认"));
		break;
	case 0x28:
		receivedMessage.append(TEXT("POSSIBLEFRIEND_MSG"));
		break;
	case 0x2A:
		receivedMessage.append(TEXT("名片"));
		break;
	case 0x2B:
		receivedMessage.append(TEXT("视频"));
		break;
	case 0x2F:
		//石头剪刀布
		receivedMessage.append(TEXT("表情"));
		break;
	case 0x30:
		receivedMessage.append(TEXT("位置"));
		break;
	case 0x31:
		//共享实时位置
		//文件
		//转账
		//链接
		receivedMessage.append(TEXT("共享实时位置、文件、转账、链接"));
		break;
	case 0x32:
		receivedMessage.append(TEXT("VOIPMSG"));
		break;
	case 0x33:
		receivedMessage.append(TEXT("微信初始化"));
		break;
	case 0x34:
		receivedMessage.append(TEXT("VOIPNOTIFY"));
		break;
	case 0x35:
		receivedMessage.append(TEXT("VOIPINVITE"));
		break;
	case 0x3E:
		receivedMessage.append(TEXT("小视频"));
		break;
	case 0x270F:
		receivedMessage.append(TEXT("SYSNOTICE"));
		break;
	case 0x2710:
		//系统消息
		//红包
		receivedMessage.append(TEXT("红包、系统消息"));
		break;
	case 0x2712:
		receivedMessage.append(TEXT("撤回消息"));
		break;
	default:
		wostringstream oss;
		oss.fill('0');
		oss << setiosflags(ios::uppercase) << setw(8) << hex << msgType;
		receivedMessage.append(TEXT("未知:0x"));
		receivedMessage.append(oss.str());
		break;
	}
	receivedMessage.append(TEXT("\r\n"));

	//dc [[[esp]] + 0x114]
	//判断是群消息还是好友消息
	//相关信息
	wstring msgSource2 = TEXT("<msgsource />\n");
	wstring msgSource = TEXT("");
	msgSource.append(GetMsgByAddress(**msgAddress + 0x168));

	if (msgSource.length() <= msgSource2.length())
	{
		receivedMessage.append(TEXT("收到好友消息:\r\n"));
		isFriendMsg = TRUE;
	}
	else
	{
		receivedMessage.append(TEXT("收到群消息:\r\n"));
		isFriendMsg = FALSE;
	}

	//好友消息
	if (isFriendMsg == TRUE)
	{
		receivedMessage.append(TEXT("好友wxid：\r\n"))
			.append(GetMsgByAddress(**msgAddress + 0x40))
			.append(TEXT("\r\n\r\n"));

		fromWxid = LPCWSTR2String(GetMsgByAddress(**msgAddress + 0x40));

	}
	else
	{
		receivedMessage.append(TEXT("群号：\r\n"))
			.append(GetMsgByAddress(**msgAddress + 0x40))
			.append(TEXT("\r\n\r\n"));

		receivedMessage.append(TEXT("消息发送者：\r\n"))
			.append(GetMsgByAddress(**msgAddress + 0x114))
			.append(TEXT("\r\n\r\n"));

		receivedMessage.append(TEXT("相关信息：\r\n"));
		receivedMessage += msgSource;
		receivedMessage.append(TEXT("\r\n\r\n"));

		fromWxid = LPCWSTR2String(GetMsgByAddress(**msgAddress + 0x40));
		senderWxid = LPCWSTR2String(GetMsgByAddress(**msgAddress + 0x114));
	}

	receivedMessage.append(TEXT("消息内容：\r\n"))
		.append(GetMsgByAddress(**msgAddress + 0x68))
		.append(TEXT("\r\n\r\n"));
	content = LPCWSTR2String(GetMsgByAddress(**msgAddress + 0x68));

	receivedMessage.append(TEXT("未知内容：\r\n"))
		.append(GetMsgByAddress(**msgAddress + 0x128))
		.append(TEXT("\r\n\r\n"));

	
	//文本框输出信息
	SetDlgItemText(getGlobalHwnd(), TEXT_RECIEVE_MSG, receivedMessage.c_str());


	//如果收到的信息是文本，开个线程数据保持到mysql
	if (msgType == 0x01) {
		thread t1(InsertData, fromWxid, senderWxid, content);
		t1.detach();
	}


}
