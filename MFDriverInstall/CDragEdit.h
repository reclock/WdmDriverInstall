#pragma once
#include <afxwin.h>
class CDragEdit :
	public CEdit
{
public:
	CDragEdit();
	~CDragEdit();
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
};

