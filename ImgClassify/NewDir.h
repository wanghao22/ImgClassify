#pragma once


// NewDir �Ի���

class NewDir : public CDialogEx
{
	DECLARE_DYNAMIC(NewDir)

public:
	NewDir(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~NewDir();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INPUT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_name;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
