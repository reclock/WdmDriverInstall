
// MFDriverInstallDlg.h: 头文件
//

#pragma once
#include "CDriverInstall.h"


// CMFDriverInstallDlg 对话框
class CMFDriverInstallDlg : public CDialogEx
{
// 构造
public:
	CMFDriverInstallDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFDRIVERINSTALL_DIALOG };
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
//	CString m_infpath;
	afx_msg void OnBnClickedButtonSelect();
	CString m_infpath;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonInstall();

public:

	CString m_hardwareID;
	CDriverInstall m_driver;
	afx_msg void OnBnClickedButtonUnstall();
};
