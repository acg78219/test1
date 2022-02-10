#pragma once
// ChatClient 对话框

#define WM_DISPLAY	WM_USER + 1

class ChatClient : public CDialogEx
{
	DECLARE_DYNAMIC(ChatClient)

public:
	ChatClient(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ChatClient();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit1();
//	CString mIP;
	CEdit mEditInput;
	CString mInput;
	CEdit mEditShowMsg;
	CString mShowMsg;

	SOCKET sockTarget;
	HANDLE hThread;

	afx_msg void OnClickedBtncon();
	afx_msg void OnClickedBtnquit();
	afx_msg void OnClickedBtnsendmsg();

	LRESULT OnDisplay(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	CString mIP;
	char* IP;
};
