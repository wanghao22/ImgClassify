
// ImgClassifyDlg.h : 头文件
//

#pragma once
#include <opencv2\opencv.hpp>
#include "NewDir.h"
#include "DirFileOption.h"
#include "afxwin.h"

#define _WIN_NAME_		"show"
#define _PARAM_DATA_	"param.dat"
#define _MAX_NUMS_		8

// CImgClassifyDlg 对话框
class CImgClassifyDlg : public CDialogEx
{
// 构造
public:
	CImgClassifyDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMGCLASSIFY_DIALOG };
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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	bool m_open_flag;
	bool m_open_thread_flag;
	bool m_del_calass_flag;
	void __init__();
	void SetImgWindow(int Dlgitem, const char * str);
	void ShowMatImg(cv::Mat src, int Dlgitem, const char * str);
	CRect m_wndRect;
	afx_msg void OnBnClickedNewClass();
	CString SelFilePath();
	afx_msg void OnBnClickedSelectProject();
	void findFile(std::string path, std::vector<std::string>& res);
	void findFile(std::string path, std::vector<std::string>& res, std::vector<std::string>& _dirs);
	int PathDevision(std::string str, std::string & path, std::string & name);
	void LoadImages(std::string fullpath);
	CString m_project_dir; std::string m_project_dir_str;
	std::vector<std::string> m_fullpath, m_path, m_name, m_dirs;
	CListBox m_unkown_list;
	cv::Mat init_mat;
	afx_msg void OnLbnSelchangeImgList();
	CComboBox m_combox_class;
	afx_msg void OnBnClickedDelClass();
	afx_msg void OnBnClickedButtonSure();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonNg();
	void DlgItemShowEnale(bool flag);
	void ThreadShowEnable(bool flag);
	int MoveMatToDir(cv::Mat src, std::string path, int index, bool show_flag = true);
	CWinThread* m_pthread;
	CWinThread* m_pthread_del;
	CWinThread* m_pthread_tvt;
	void closeThread();
	HACCEL m_hAccel;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonReturn();
	afx_msg void OnBnClickedButtonClear();
	std::vector<std::string> m_pathc, m_namec;
	void OnSortMove(int dlgItemID);
	afx_msg void OnBnClickedAddtoOther1();
	afx_msg void OnBnClickedAddtoOther2();
	afx_msg void OnBnClickedAddtoOther3();
	afx_msg void OnBnClickedAddtoOther4();
	afx_msg void OnBnClickedAddtoOther5();
	afx_msg void OnBnClickedAddtoOther6();
	afx_msg void OnBnClickedAddtoOther7();
	afx_msg void OnBnClickedAddtoOther8();
	void Button_init();
	bool m_train_val_test_falg;
	afx_msg void OnBnClickedTrainValTest();
	CButton m_is_color;
	BOOL m_is_color_flag;
};
