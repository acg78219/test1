
// ServerDlg.h: 头文件
//

#pragma once


// CServerDlg 对话框
class CServerDlg : public CDialogEx
{
// 构造
public:
	CServerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedBtnstart();
	afx_msg void OnClickedBtnsendmsg();
	afx_msg void OnClickedBtnquit();
	afx_msg void OnDestroy();
	LRESULT OnDisplay(WPARAM wParam, LPARAM lParam);

	CString mInput;
	CString mShowMsg;

	HANDLE mThread;
	SOCKET sockCli;
	SOCKET sockSrv;
	CEdit mEditInput;
	CEdit mEditShowMsg;
};
