
// MainDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Main.h"
#include "MainDlg.h"
#include "afxdialogex.h"

#include "ChatClient.h"
#include "GetHttp.h"

// 引入发送 post 请求的接口
#include "utils.h"

#include <string>
#include <string.h>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

size_t WriteMemoryCallback(void* ptr, size_t size, size_t nmenb, void* data) {
	size_t realsize = size * nmenb;
	struct postData* mem = (struct postData*)data;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory) {
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	return realsize;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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
public:
	afx_msg void OnDestroy();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMainDlg 对话框



CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_DIALOG, pParent)
	, mPasswd(_T(""))
	, mUser(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITPASSWD, mPasswd);
	DDX_Control(pDX, IDC_EDITPASSWD, mEditPasswd);
	DDX_Control(pDX, IDC_EDITUSER, mEditUser);
	DDX_Text(pDX, IDC_EDITUSER, mUser);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSTART, &CMainDlg::OnClickedBtnstart)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序

BOOL CMainDlg::OnInitDialog()
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

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMainDlg::OnPaint()
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
		//CDialogEx::OnPaint();
		CPaintDC	dc(this);
		CRect	rect;
		GetClientRect(&rect);

		CDC	dcBmp;
		dcBmp.CreateCompatibleDC(&dc);
		CBitmap	bmpBackgroup;
		bmpBackgroup.LoadBitmapW(IDB_BITMAP2);

		BITMAP	m_bitmap;
		bmpBackgroup.GetBitmap(&m_bitmap);
		CBitmap* pbmpOld = dcBmp.SelectObject(&bmpBackgroup);

		// 调整图片位置
		dc.StretchBlt(rect.Width() / 2 - 114, 25, 228, 94, &dcBmp, 0, 0,
			m_bitmap.bmWidth, m_bitmap.bmHeight, SRCCOPY);
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMainDlg::OnClickedBtnstart()
{
	// 点击按钮——》验证成功——》跳转到聊天室对话框
	// 
	//验证流程：
	// 1. 获取输入框信息
	// 2. 封装成一个 post 请求发给指定地址
	// 3. 获取判断返回值
	UpdateData(TRUE);

	if (mUser == "" && mPasswd == "") {
		MessageBox(_T("请输入用户名和密码！"));
		return;
	}

	// 使用 libcurl 发送 post 请求
		// 首先获取本机 mac（header需要）
	GetHttp http1;
	http1.GetMac();
	CString tmpMyMac = http1.mac;
		// 头部拼接
	string tmpHeader = "onlymark:";
	tmpHeader += CT2A(tmpMyMac);

		// 表单 body 拼接
	string user = "username=";
	user += CT2A(mUser);
	user += "&";
	string passwd = "password=";
	passwd += CT2A(mPasswd);
	string tmpJsonData = user + passwd;

	// 创建 sendPost 对象，发送任务交给这个类处理
	//sendPost myPost;
	string url = "http://test3.yj81.com/api/v3/user/login";
	string header = tmpHeader;
	string jsonData = tmpJsonData;

	// 此方案有个问题，就是返回的是结果的地址，但是结果超过作用域被销毁，所以实际上拿不到
	//postData myData = myPost.post(url, header, jsonData);	// 封装好请求后，执行 post，返回数据
	CURL* handle = curl_easy_init();

	auto res = curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

	// 封装头部
	curl_slist* hList = NULL;
	hList = curl_slist_append(hList, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
	hList = curl_slist_append(hList, "Accept:application/json, text/javascript, */*; q=0.01");
	hList = curl_slist_append(hList, "Accept-Language:zh-CN,zh;q=0.8");
	hList = curl_slist_append(hList, header.c_str());
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, hList);

	// 设置回调函数和数据接收
	postData myData;
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &myData);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);	// 启用后会汇报所有信息

	// 设置表单数据
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, jsonData.c_str());
	curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, jsonData.size());

	// 其他设置
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(handle, CURLOPT_POST, 1L);

	// 发送 post 请求
	res = curl_easy_perform(handle);

	// 将结果 code 保存到数据中
	long res_code = 0;
	res = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res_code);
	myData.code = res_code;
	myData.res = res;

	// 关闭资源
	curl_slist_free_all(hList);
	curl_easy_cleanup(handle);

	if ((myData.res == CURLE_OK) && (myData.code == 200)) {
		// 解析数据--》这里我们截取第一个字段（code:XXX）
		char delims[] = ",";
		char* tokPtr;
		char* code = strtok_s(myData.memory, delims, &tokPtr);
		string tmp = code;
		string strNum = "";

		// 用一个简单的方法获取数字
		for (auto ch : tmp) {
			if (isdigit(ch)) {
				strNum += ch;
			}
		}
		// 判断 code 数字
		if (strNum == "0") {
			this->ShowWindow(SW_HIDE);
			ChatClient clientDlg;
			clientDlg.DoModal();	// 子对话框结束后返回
			this->ShowWindow(SW_SHOW);
		}
		else {
			MessageBox(_T("身份验证失败！"));
			return;
		}
	}
	else {
		MessageBox(_T("post 发送失败！"));
		return;
	}
}


void CAboutDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}
