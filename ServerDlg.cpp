
// ServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4996)


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

DWORD WINAPI srvThread (LPVOID lpParamer) {
	CServerDlg* srvDlg = (CServerDlg*)lpParamer;
	SOCKADDR_IN cliAddr;
	int len = sizeof(SOCKADDR);
	fd_set fs;
	timeval tv = { 60, 5000 };
	int selRet = 0;

	// 两层select:
	// 1. select 监听客户连接到来
	// 2. select 监听客户的读事件
SELLOOP:
	FD_ZERO(&fs);
	FD_SET(srvDlg->sockSrv, &fs);
	selRet = select(1, &fs, NULL, NULL, &tv);
	// 大部分情况是处于空闲状态，等待客户端连接到来，所以不断循环
	if (selRet <= 0) {
		Sleep(5);
		goto SELLOOP;
	}

	// 客户端连接到来，accept 接受连接，并把返回值赋值给 sockCli
	if ((srvDlg->sockCli = accept(srvDlg->sockSrv, (SOCKADDR*)&cliAddr, &len)) == SOCKET_ERROR) {
		srvDlg->mShowMsg += "accept 失败!\r\n";
		PostMessage((HWND)srvDlg->m_hWnd, WM_DISPLAY, NULL, NULL);
		return 1;
	}

	srvDlg->mShowMsg += "已于对方建立连接!\r\n";
	PostMessage((HWND)srvDlg->m_hWnd, WM_DISPLAY, NULL, NULL);
		
	// 线程死循环处于客户端事务
	while (true) {
		FD_ZERO(&fs);
		FD_SET(srvDlg->sockCli, &fs);
		int tmpRet = select(1, &fs, NULL, NULL, &tv);
		// select 有三种情况：
		// 1. 正常监听到读写事件——处于读写通信事件
		if (tmpRet == 1) {
			// recv 也有几种情况：
			// 1. 对端关闭连接——正常和异常都一起处理
			char recvBuf[2048] = "";
			int recvRet = recv(srvDlg->sockCli, recvBuf, sizeof(recvBuf), 0);
			if (recvRet == 0 || recvRet == SOCKET_ERROR) {
				srvDlg->mShowMsg += "对端关闭连接！\r\n";
				PostMessage((HWND)srvDlg->m_hWnd, WM_DISPLAY, NULL, NULL);
				closesocket(srvDlg->sockCli);
				// 如果对方关闭连接，则返回第一层 select 去监听连接socket
				goto SELLOOP;
			}
			// 2. 正常接受到数据，返回字节数——显示发来的数据
			else {
				CString tmp = _T("对端：");
				tmp += recvBuf;
				tmp += "\r\n";
				srvDlg->mShowMsg += tmp;
				PostMessage((HWND)srvDlg->m_hWnd, WM_DISPLAY, NULL, NULL);
			}
		}
		// 2. 对端长时间无反应——显示错误信息，但是无退出循环
		else if (tmpRet == 0) {
			srvDlg->mShowMsg += "等待超时！\r\n";
			PostMessage((HWND)srvDlg->m_hWnd, WM_DISPLAY, NULL, NULL);
		}
		// 3. select 发生错误——显示错误信息，但是不退出循环
		else {
			srvDlg->mShowMsg += "select 错误！\r\n";
			PostMessage((HWND)srvDlg->m_hWnd, WM_DISPLAY, NULL, NULL);
		}
		Sleep(5);
	}
	return 0;
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerDlg 对话框



CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
	, mInput(_T(""))
	, mShowMsg(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITINPUT, mInput);
	DDX_Text(pDX, IDC_EDITSHOWMSG, mShowMsg);
	DDX_Control(pDX, IDC_EDITINPUT, mEditInput);
	DDX_Control(pDX, IDC_EDITSHOWMSG, mEditShowMsg);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSTART, &CServerDlg::OnClickedBtnstart)
	ON_BN_CLICKED(IDC_BTNSENDMSG, &CServerDlg::OnClickedBtnsendmsg)
	ON_BN_CLICKED(IDC_BTNQUIT, &CServerDlg::OnClickedBtnquit)
	ON_MESSAGE(WM_DISPLAY, OnDisplay)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CServerDlg 消息处理程序

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		mShowMsg += "winsock 启动失败！\r\n";
		UpdateData(FALSE);
		WSACleanup();
		return 1;
	}

	sockCli = INVALID_SOCKET;
	sockSrv = INVALID_SOCKET;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerDlg::OnClickedBtnstart()
{
	sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	sockCli = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSrv == INVALID_SOCKET || sockCli == INVALID_SOCKET) {
		mShowMsg += "socket初始化失败！\r\n";
		UpdateData(FALSE);
		closesocket(sockSrv);
		closesocket(sockCli);
		return;
	}

	SOCKADDR_IN sockAddr;
	//sockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sockAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(12345);
	sockAddr.sin_family = AF_INET;

	int ret = bind(sockSrv, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	if (ret != 0) {
		mShowMsg += "bind失败！\r\n";
		UpdateData(FALSE);
		closesocket(sockSrv);
		closesocket(sockCli);
		return;
	}

	ret = listen(sockSrv, 5);
	if (ret == SOCKET_ERROR) {
		mShowMsg += "listen 失败！\r\n";
		UpdateData(FALSE);
		closesocket(sockSrv);
		closesocket(sockCli);
		return;
	}
	mShowMsg += "启动成功！\r\n";
	UpdateData(FALSE);

	mThread = CreateThread(NULL, 0, srvThread, (LPVOID)this, 0, NULL);
	if (mThread == NULL) {
		mShowMsg += "创建线程失败！\r\n";
		UpdateData(FALSE);
		closesocket(sockSrv);
		closesocket(sockCli);
	}
	return;
}


void CServerDlg::OnClickedBtnsendmsg()
{
	UpdateData(TRUE);
	if (!mInput.IsEmpty()) {
		USES_CONVERSION;
		mInput += "\0";
		char* sendBuf = T2A(mInput);

		int ret = send(sockCli, sendBuf, strlen(sendBuf), 0);
		if (send <= 0) {
			mShowMsg += "发送失败！\r\n";
			UpdateData(FALSE);
			return;
		}
		else if (ret > 0) {
			mShowMsg += "本机：";
			mShowMsg += sendBuf;
			mShowMsg += "\r\n";
			mInput = "";
			UpdateData(FALSE);
		}
	}
}


void CServerDlg::OnClickedBtnquit()
{
	shutdown(sockCli, SD_RECEIVE);
	closesocket(sockCli);
	closesocket(sockSrv);
	CloseHandle(mThread);
	//CDialogEx::OnCancel();
}


void CServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	closesocket(sockSrv);
	closesocket(sockCli);
	CloseHandle(mThread);
}

LRESULT CServerDlg::OnDisplay(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}
