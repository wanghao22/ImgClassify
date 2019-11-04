// NewDir.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgClassify.h"
#include "NewDir.h"
#include "afxdialogex.h"


// NewDir 对话框

IMPLEMENT_DYNAMIC(NewDir, CDialogEx)

NewDir::NewDir(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_INPUT, pParent)
	, m_name(_T(""))
{

}

NewDir::~NewDir()
{
}

void NewDir::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_name);
}


BEGIN_MESSAGE_MAP(NewDir, CDialogEx)
	ON_BN_CLICKED(IDOK, &NewDir::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &NewDir::OnBnClickedCancel)
END_MESSAGE_MAP()


// NewDir 消息处理程序


void NewDir::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	for (int i = 0; i < m_name.GetLength(); i++)
	{
		if (m_name[i] <= L'Z'&&m_name[i] >= L'A' ||
			m_name[i] <= L'z'&&m_name[i] >= L'a' ||
			m_name[i] <= L'9'&&m_name[i] >= L'0' ||
			m_name[i] == L'-' || m_name[i] == L'_')
			continue;
		else
			return;
	}

	CDialogEx::OnOK();
}


void NewDir::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	CDialogEx::OnCancel();
}
