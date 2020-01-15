
// MFDriverInstallDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MFDriverInstall.h"
#include "MFDriverInstallDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <windows.h>
#include <newdev.h>
#include <SetupAPI.h>
#include <locale.h>
#include <tchar.h>
#include <string.h>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#pragma comment(lib,"newdev.lib")
#pragma comment(lib,"setupapi.lib")


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


// CMFDriverInstallDlg 对话框



CMFDriverInstallDlg::CMFDriverInstallDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFDRIVERINSTALL_DIALOG, pParent)
	, m_infpath(_T(""))
	, m_hardwareID(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFDriverInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Text(pDX, IDC_EDIT1, m_infpath);
	DDX_Text(pDX, IDC_EDIT1, m_infpath);
	DDX_Text(pDX, IDC_EDIT2, m_hardwareID);
}

BEGIN_MESSAGE_MAP(CMFDriverInstallDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CMFDriverInstallDlg::OnBnClickedButtonSelect)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON_INSTALL, &CMFDriverInstallDlg::OnBnClickedButtonInstall)
	ON_BN_CLICKED(IDC_BUTTON_UNSTALL, &CMFDriverInstallDlg::OnBnClickedButtonUnstall)
END_MESSAGE_MAP()
;

// CMFDriverInstallDlg 消息处理程序

void ShowErrMsg(DWORD Count, LPCTSTR szMsg)
{
	printf("%d\n%s", &Count, szMsg);
}


BOOL CMFDriverInstallDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);

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

	DragAcceptFiles(TRUE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFDriverInstallDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFDriverInstallDlg::OnPaint()
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

HCURSOR CMFDriverInstallDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFDriverInstallDlg::OnBnClickedButtonSelect()
{
	CString filepath;
	CFileDialog filedlg(TRUE, "INF File(*.inf)|*.inf", NULL,OFN_HIDEREADONLY | OFN_READONLY,"INF File(*.inf)|*.inf||", NULL);
	UINT nret =  filedlg.DoModal();
	if (nret == IDOK)
	{
		filepath = filedlg.GetPathName();
	}

	m_infpath = filepath;

	FILE *pInf;
	errno_t err;
	CHAR cbuf[64] = { 0 };

	if ((err = fopen_s(&pInf, m_infpath.GetBuffer(0), "r")))
	{
		_tprintf(_T("can nor open file %s\n", g_infPath));
		return;
	}

	m_driver.GetINFData(pInf);

	m_hardwareID = m_driver.g_strHardware[0];
	fclose(pInf);

	UpdateData(FALSE);

}


void CMFDriverInstallDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);//取得被拖动文件的数目
	for (int i = 0; i < DropCount; i++)
	{
		CHAR wcStr[MAX_PATH];
		DragQueryFile(hDropInfo, i, wcStr, MAX_PATH);//获得拖曳的第i个文件的文件名
		m_infpath = wcStr;
	}
	
	DragFinish(hDropInfo);  //拖放结束后,释放内存

	UpdateData(FALSE);

	CDialogEx::OnDropFiles(hDropInfo);
}


BOOL CMFDriverInstallDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CMFDriverInstallDlg::OnBnClickedButtonInstall()
{
	BOOL bret =  m_driver.StartInstallWDMDriver(m_infpath);

	if(bret)
	{
		AfxMessageBox("驱动安装成功");
	}
	else
	{
		AfxMessageBox("安装失败");
	}
}


void CMFDriverInstallDlg::OnBnClickedButtonUnstall()
{
	m_driver.UninstallWdmDriver(m_hardwareID);
}
