// ChatClient.cpp: 实现文件
//

#include "pch.h"
#include "Main.h"
#include "ChatClient.h"
#include "afxdialogex.h"
#include <WS2tcpip.h>


// ChatClient 对话框

IMPLEMENT_DYNAMIC(ChatClient, CDialogEx)

DWORD WINAPI TarThread(LPVOID lpParameter) {
	ChatClient* cliDlg = (ChatClient*)lpParameter;
	fd_set fs;
	timeval tv = { 60, 5000 };
	int selRet = 0;

	while (true) {
		FD_ZERO(&fs);
		FD_SET(cliDlg->sockTarget, &fs);
		selRet = select(1, &fs, NULL, NULL, &tv);
		// 读写事件触发
		if (selRet == 1) {
			char recvBuf[2048] = "";
			int recvRet = recv(cliDlg->sockTarget, recvBuf, sizeof(recvBuf), 0);
			if (recvRet == SOCKET_ERROR || recvRet == 0) {
				cliDlg->mShowMsg += "对端断开连接！\r\n";
				PostMessage((HWND)(cliDlg->m_hWnd), WM_DISPLAY, NULL, NULL);
				closesocket(cliDlg->sockTarget);
				break;
			}
			// 正常接受数据
			else
			{
				CString tmp;
				tmp = "对端：";
				tmp += recvBuf;
				tmp += "\r\n";
				cliDlg->mShowMsg += tmp;
				PostMessage((HWND)(cliDlg->m_hWnd), WM_DISPLAY, NULL, NULL);
			}
		}
		// select 超时
		else if (selRet == 0) {
			cliDlg->mShowMsg += "select 超时！\r\n";
			PostMessage((HWND)(cliDlg->m_hWnd), WM_DISPLAY, NULL, NULL);
		}
		else {
			cliDlg->mShowMsg += "select 错误！\r\n";
			PostMessage((HWND)(cliDlg->m_hWnd), WM_DISPLAY, NULL, NULL);
		}
		Sleep(5);
	}
	return 0;
}

ChatClient::ChatClient(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, mInput(_T(""))
	, mShowMsg(_T(""))
	, mIP(_T(""))
	, IP(nullptr)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		mShowMsg += "Winsock 无法初始化！\r\n";
		UpdateData(FALSE);
		WSACleanup();
		return;
	}

	sockTarget = INVALID_SOCKET;

}

ChatClient::~ChatClient()
{
}

void ChatClient::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITINPUT, mEditInput);
	DDX_Text(pDX, IDC_EDITINPUT, mInput);
	DDX_Control(pDX, IDC_EDITSHOWMSG, mEditShowMsg);
	DDX_Text(pDX, IDC_EDITSHOWMSG, mShowMsg);
	DDX_Text(pDX, IDC_EDITIP, mIP);
}

BEGIN_MESSAGE_MAP(ChatClient, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT2, &ChatClient::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BTNCON, &ChatClient::OnClickedBtncon)
	ON_BN_CLICKED(IDC_BTNQUIT, &ChatClient::OnClickedBtnquit)
	ON_BN_CLICKED(IDC_BTNSENDMSG, &ChatClient::OnClickedBtnsendmsg)
	ON_MESSAGE(WM_DISPLAY, OnDisplay)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// ChatClient 消息处理程序

void ChatClient::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void ChatClient::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void ChatClient::OnClickedBtncon()
{
	// 与对方 IP 进行连接通信
	// 初始化 socket
	sockTarget = socket(AF_INET, SOCK_STREAM, 0);
	if (sockTarget == INVALID_SOCKET) {
		mShowMsg += "创建 socket 失败！\r\n";
		UpdateData(FALSE);
		return;
	}
	
	UpdateData(TRUE);
	SOCKADDR_IN addrTarget;
	USES_CONVERSION;
	// 将输入框 IP 转换为 char*
	mIP += "\0";
	IP = T2A(mIP);
	inet_pton(AF_INET, IP, &addrTarget.sin_addr.S_un.S_addr);
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(12345);

	int ret = connect(sockTarget, (SOCKADDR*)&addrTarget, sizeof(SOCKADDR));
	if (ret != 0) {
		closesocket(sockTarget);
		mShowMsg += "connect 失败!\r\n";
		UpdateData(FALSE);
		return;
	}

	mShowMsg += "connect成功！\r\n";
	UpdateData(FALSE);

	hThread = CreateThread(NULL, 0, TarThread, (LPVOID)this, 0, NULL);
	if (hThread == NULL) {
		mShowMsg += "创建线程失败!\r\n";
		UpdateData(FALSE);
		closesocket(sockTarget);
		return;
	}
	else return;
}

void ChatClient::OnClickedBtnquit()
{
	shutdown(sockTarget, SD_RECEIVE);
	CloseHandle(hThread);
	closesocket(sockTarget);
	mShowMsg += "通信关闭！\r\n";
	UpdateData(FALSE);

	//CDialogEx::OnCancel();
}

void ChatClient::OnClickedBtnsendmsg()
{
	UpdateData(TRUE);
	if (!mInput.IsEmpty()) {
		USES_CONVERSION;
		mInput += "\0";
		char* sendBuf = T2A(mInput);

		int ret = send(sockTarget, sendBuf, strlen(sendBuf), 0);
		if (ret == 0) {
			mShowMsg += "发送信息失败！\r\n";
			UpdateData(FALSE);
		}
		// 发送成功就更新聊天室和清空输入框
		else if (ret > 0) {
			mShowMsg += "本机：";
			mShowMsg += sendBuf;
			mShowMsg += "\r\n";
			mInput = "";
			UpdateData(FALSE);
		}
	}
}

LRESULT ChatClient::OnDisplay(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}


void ChatClient::OnDestroy()
{

	CDialogEx::OnDestroy();
	CloseHandle(hThread);
	closesocket(sockTarget);
}

