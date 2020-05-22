
// ImgClassifyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ImgClassify.h"
#include "ImgClassifyDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
using namespace std;

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


// CImgClassifyDlg 对话框

UINT LoadImage_Thread(PVOID pParam)
{
	CImgClassifyDlg* pWnd = (CImgClassifyDlg*)pParam;
	pWnd->m_open_thread_flag = true;
	pWnd->m_fullpath.clear();
	pWnd->m_path.clear();
	pWnd->m_name.clear();
	std::vector<std::string> temp;
	pWnd->findFile(pWnd->m_project_dir_str, temp, pWnd->m_dirs);
	Mat src;
	string paths, names, types;

	CProgressCtrl myProCtrl;
	CRect rect, proRect;
	GetClientRect(pWnd->m_hWnd, &rect);
	proRect.left = rect.left + rect.Width() / 2 - 100;
	proRect.top = rect.top + rect.Height() / 2 - 20;
	proRect.right = rect.right - rect.Width() / 2 + 100;
	proRect.bottom = rect.bottom - rect.Height() / 2 + 20;
	//WS_CHILD|WS_VISIBLE|PBS_SMOOTHREVERSE
	myProCtrl.Create(WS_VISIBLE, proRect, pWnd, 99); //创建位置、大小
	myProCtrl.SetRange(0, 100);

	for (size_t i = 0; i < temp.size(); i++)
	{
		src = imread(temp[i], pWnd->m_is_color_flag);
		if (src.empty())
			continue;
		if (pWnd->PathDevision(temp[i], paths, names) != 0)
			continue;
		pWnd->m_fullpath.push_back(temp[i]);
		pWnd->m_path.push_back(paths);
		pWnd->m_name.push_back(names);
		int pos = (i + 1) * 100 / temp.size();
		pWnd->m_unkown_list.AddString(CString(names.c_str()));
		myProCtrl.SetPos(pos);
	}
	//for (size_t i = 0; i < pWnd->m_name.size(); i++)
	for (size_t i = 0; i < pWnd->m_dirs.size(); i++)
		pWnd->m_combox_class.AddString(CString(pWnd->m_dirs[i].c_str()));
	if (pWnd->m_dirs.size() > 0)
		pWnd->m_combox_class.SetCurSel(0);
	pWnd->ThreadShowEnable(true);
	pWnd->SetDlgItemText(IDC_SELECT_PROJECT, L"关闭工程");
	pWnd->m_open_flag = true;

	pWnd->m_open_thread_flag = false;
	return 0;
}

UINT DelClass_Thread(PVOID pParam)
{
	CImgClassifyDlg* pWnd = (CImgClassifyDlg*)pParam;
	pWnd->ThreadShowEnable(false);
	pWnd->m_del_calass_flag = true;
	CString cstr;
	pWnd->GetDlgItemText(IDC_COMBO_CLASS, cstr);
	string path = CW2A(pWnd->m_project_dir + L"\\" + cstr);
	int index = pWnd->m_combox_class.GetCurSel();
	pWnd->m_combox_class.DeleteString(index);
	std::vector<std::string> temp;
	pWnd->findFile(path, temp);
	Mat src;
	string paths, names, types;

	CProgressCtrl myProCtrl;
	CRect rect, proRect;
	GetClientRect(pWnd->m_hWnd, &rect);
	proRect.left = rect.left + rect.Width() / 2 - 100;
	proRect.top = rect.top + rect.Height() / 2 - 20;
	proRect.right = rect.right - rect.Width() / 2 + 100;
	proRect.bottom = rect.bottom - rect.Height() / 2 + 20;
	//WS_CHILD|WS_VISIBLE|PBS_SMOOTHREVERSE
	myProCtrl.Create(WS_VISIBLE, proRect, pWnd, 99); //创建位置、大小
	myProCtrl.SetRange(0, 100);

	for (size_t i = 0; i < temp.size(); i++)
	{
		src = imread(temp[i], pWnd->m_is_color_flag);
		if (src.empty())
			continue;
		if (pWnd->PathDevision(temp[i], paths, names) != 0)
			continue;
		DirFile::_DeleteFile(temp[i]);
		string newfullpath = DirFile::DirAddSubdir(pWnd->m_project_dir_str, names + types);
		imwrite(newfullpath, src);
		pWnd->m_fullpath.push_back(newfullpath);
		pWnd->m_unkown_list.AddString(CString((names + types).c_str()));
		pWnd->m_path.push_back(paths);
		pWnd->m_name.push_back(names);
		int pos = (i + 1) * 100 / temp.size();
		myProCtrl.SetPos(pos);
	}
	DirFile::DeleteDir(path);
	if (pWnd->m_combox_class.GetCount() > 0)
		pWnd->m_combox_class.SetCurSel(0);
	else
		pWnd->SetDlgItemText(IDC_COMBO_CLASS, L"");
	pWnd->ThreadShowEnable(true);
	pWnd->m_del_calass_flag = false;
	return 0;
}

UINT TrainValTest_Thread(PVOID pParam)
{
	CImgClassifyDlg* pWnd = (CImgClassifyDlg*)pParam;
	pWnd->ThreadShowEnable(false);
	pWnd->m_train_val_test_falg = true;

	int sum = pWnd->m_unkown_list.GetCount();
	int cnt = 1;

	CProgressCtrl myProCtrl;
	CRect rect, proRect;
	GetClientRect(pWnd->m_hWnd, &rect);
	proRect.left = rect.left + rect.Width() / 2 - 100;
	proRect.top = rect.top + rect.Height() / 2 - 20;
	proRect.right = rect.right - rect.Width() / 2 + 100;
	proRect.bottom = rect.bottom - rect.Height() / 2 + 20;
	//WS_CHILD|WS_VISIBLE|PBS_SMOOTHREVERSE
	myProCtrl.Create(WS_VISIBLE, proRect, pWnd, 99); //创建位置、大小
	myProCtrl.SetRange(0, 100);

	while (pWnd->m_unkown_list.GetCount()>0)
	{
		int index = 0;
		int t = rand() % 100;
		string temp;
		if (t >= 0 && t < 70)
			temp = ".\\train";
		else if (t >= 70 && t < 85)
			temp = ".\\val";
		else if (t >= 85 && t < 100)
			temp = ".\\test";
		else
		{
			CString ccc; ccc.Format(L"%d", t);
			AfxMessageBox(ccc);
		}
		temp = DirFile::DirAddSubdir(pWnd->m_project_dir_str, temp);
		string fullpath = DirFile::DirAddSubdir(temp, pWnd->m_name[index]);
		Mat src;
		src = imread(pWnd->m_fullpath[index], pWnd->m_is_color_flag);
		if (!src.empty())
		{
			pWnd->m_pathc.push_back(temp);
			pWnd->m_namec.push_back(pWnd->m_name[index]);
			int rc = pWnd->MoveMatToDir(src, fullpath, index, false);
			if (rc != 0)
				break;
		}
		cnt++;
		int pos = cnt * 100 / sum;
		myProCtrl.SetPos(pos);
	}
	pWnd->ShowMatImg(pWnd->init_mat, IDC_SHOW_IMG, _WIN_NAME_);
	pWnd->ThreadShowEnable(true);
	pWnd->m_del_calass_flag = false;
	return 0;
}

CImgClassifyDlg::CImgClassifyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMGCLASSIFY_DIALOG, pParent)
	, m_open_thread_flag(false)
	, m_open_flag(false)
	, m_del_calass_flag(false)
	, m_train_val_test_falg(false)
	, m_project_dir_str("")
	, m_is_color_flag(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CImgClassifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMG_LIST, m_unkown_list);
	DDX_Control(pDX, IDC_COMBO_CLASS, m_combox_class);
	DDX_Control(pDX, IDC_IS_COLOR, m_is_color);
}

BEGIN_MESSAGE_MAP(CImgClassifyDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_NEW_CLASS, &CImgClassifyDlg::OnBnClickedNewClass)
	ON_BN_CLICKED(IDC_SELECT_PROJECT, &CImgClassifyDlg::OnBnClickedSelectProject)
	ON_LBN_SELCHANGE(IDC_IMG_LIST, &CImgClassifyDlg::OnLbnSelchangeImgList)
	ON_BN_CLICKED(IDC_DEL_CLASS, &CImgClassifyDlg::OnBnClickedDelClass)
	ON_BN_CLICKED(IDC_BUTTON_SURE, &CImgClassifyDlg::OnBnClickedButtonSure)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CImgClassifyDlg::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_NG, &CImgClassifyDlg::OnBnClickedButtonNg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_RETURN, &CImgClassifyDlg::OnBnClickedButtonReturn)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CImgClassifyDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_ADDTO_OTHER1, &CImgClassifyDlg::OnBnClickedAddtoOther1)
	ON_BN_CLICKED(IDC_ADDTO_OTHER2, &CImgClassifyDlg::OnBnClickedAddtoOther2)
	ON_BN_CLICKED(IDC_ADDTO_OTHER3, &CImgClassifyDlg::OnBnClickedAddtoOther3)
	ON_BN_CLICKED(IDC_ADDTO_OTHER4, &CImgClassifyDlg::OnBnClickedAddtoOther4)
	ON_BN_CLICKED(IDC_ADDTO_OTHER5, &CImgClassifyDlg::OnBnClickedAddtoOther5)
	ON_BN_CLICKED(IDC_ADDTO_OTHER6, &CImgClassifyDlg::OnBnClickedAddtoOther6)
	ON_BN_CLICKED(IDC_ADDTO_OTHER7, &CImgClassifyDlg::OnBnClickedAddtoOther7)
	ON_BN_CLICKED(IDC_ADDTO_OTHER8, &CImgClassifyDlg::OnBnClickedAddtoOther8)
	ON_BN_CLICKED(IDC_TRAIN_VAL_TEST, &CImgClassifyDlg::OnBnClickedTrainValTest)
END_MESSAGE_MAP()


// CImgClassifyDlg 消息处理程序

BOOL CImgClassifyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	ShowWindow(SW_MAXIMIZE);

	// TODO: 在此添加额外的初始化代码
	__init__();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CImgClassifyDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CImgClassifyDlg::OnPaint()
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
HCURSOR CImgClassifyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CImgClassifyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	for (int i = 1000; i < 1040; i++)//因为是多个控件，所以这里用了循环
	{
		CWnd *pWnd = GetDlgItem(i);
		if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())  //判断是否为空，因为对话框创建时会调用此函数，而当时控件还未创建
		{
			CRect rect;   //获取控件变化前的大小 
			pWnd->GetWindowRect(&rect);
			ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标
			rect.left = rect.left*cx / m_wndRect.Width();//调整控件大小
			rect.right = rect.right*cx / m_wndRect.Width();
			rect.top = rect.top*cy / m_wndRect.Height();
			rect.bottom = rect.bottom*cy / m_wndRect.Height();
			pWnd->MoveWindow(rect);//设置控件大小 
		}
	}
	//重新获得窗口尺寸
	GetClientRect(&m_wndRect);
}


void CImgClassifyDlg::OnClose()
{
	// TODO: m在此添加消息处理程序代码和/或调用默认值
	closeThread();

	CDialogEx::OnClose();
}


void CImgClassifyDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	closeThread();

	CDialogEx::OnOK();
}


void CImgClassifyDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	closeThread();

	CDialogEx::OnCancel();
}

void CImgClassifyDlg::__init__()
{
	SetImgWindow(IDC_SHOW_IMG, _WIN_NAME_);
	//获得窗口尺寸
	GetClientRect(&m_wndRect);
	init_mat = Mat::Mat(500, 500, CV_8UC1, cvScalarAll(0xff));

	Button_init();
	srand(int(time(0)));
	DlgItemShowEnale(false);

	ShowMatImg(init_mat, IDC_SHOW_IMG, _WIN_NAME_);
	m_hAccel = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	SetTimer(1, 1000, NULL);
}

//初始化用，确定窗口控件和窗口名称
void CImgClassifyDlg::SetImgWindow(int Dlgitem, const char* str)
{
	namedWindow(str, WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle(str);
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(Dlgitem)->m_hWnd);  //picture控件
	::ShowWindow(hParent, SW_HIDE);
}

//显示图像
void CImgClassifyDlg::ShowMatImg(cv::Mat src, int Dlgitem, const char* str)
{
	if (src.empty())
	{
		return;
	}
	cv::Mat bgr;
	if (src.channels() == 3)
		bgr = src.clone();
	else if (src.channels() == 1)
		cvtColor(src, bgr, CV_GRAY2BGR);
	else if (src.channels() == 4)
		cvtColor(src, bgr, CV_BGRA2BGR);
	else
		return;
	cv::Mat temp;
	CRect m_rect;
	GetDlgItem(Dlgitem)->GetClientRect(&m_rect);
	CvSize window_size = cvSize(m_rect.Width(), m_rect.Height());
	double width = window_size.width;
	double scale = (double)src.rows / (double)src.cols;
	double height = width*scale;
	if (width > window_size.width || height > window_size.height)
	{
		height = window_size.height;
		scale = (double)src.cols / (double)src.rows;
		width = height*scale;
		if (width > window_size.width || height > window_size.height)
		{
			return;
		}
	}
	cv::resize(bgr, temp, cvSize((int)width, (int)height));
	cv::Mat dst = cv::Mat::Mat(window_size, CV_8UC3, cvScalarAll(0xff));
	int x = (dst.cols - temp.cols) / 2;
	int y = (dst.rows - temp.rows) / 2;
	cv::Mat roi = dst(cvRect(x, y, temp.cols, temp.rows));
	temp.copyTo(roi);
	cv::imshow(str, dst);
}

//新建
void CImgClassifyDlg::OnBnClickedNewClass()
{
	// TODO: 在此添加控件通知处理程序代码
	NewDir dirdlg;
	if (dirdlg.DoModal() == IDOK)
	{
		CString cstr = dirdlg.m_name;
		CString all = m_project_dir + L"\\" + cstr;
		int rc = DirFile::CreateDir(all);
		m_combox_class.AddString(cstr);
		m_combox_class.SetCurSel(0);
	}
}

CString CImgClassifyDlg::SelFilePath()
{
	TCHAR           szFolderPath[MAX_PATH] = { 0 };
	CString         strFolderPath = TEXT("");

	BROWSEINFO      sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = _T("请选择图像路径");
	sInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_DONTGOBELOWDOMAIN;
	sInfo.lpfn = NULL;

	// 显示文件夹选择对话框  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// 取得文件夹名  
		if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
		{
			strFolderPath = szFolderPath;
		}
	}
	if (lpidlBrowse != NULL)
	{
		::CoTaskMemFree(lpidlBrowse);
	}

	return strFolderPath;
}

//打开/关闭
void CImgClassifyDlg::OnBnClickedSelectProject()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_open_flag)
	{
		while (m_unkown_list.GetCount()!=0)
			m_unkown_list.DeleteString(0);
		while (m_combox_class.GetCount() != 0)
			m_combox_class.DeleteString(0);
		m_fullpath.clear();
		m_path.clear();
		m_name.clear();
		m_project_dir = L"";
		m_project_dir_str = "";
		DlgItemShowEnale(false);
		SetDlgItemText(IDC_SELECT_PROJECT, L"打开工程");
		m_open_flag = false;
		ShowMatImg(init_mat, IDC_SHOW_IMG, _WIN_NAME_);
		GetDlgItem(IDC_IS_COLOR)->EnableWindow(true);
	}
	else
	{
		m_is_color_flag = m_is_color.GetCheck();
		ThreadShowEnable(false);
		m_project_dir = SelFilePath();
		if (m_project_dir != L"")
		{
			m_project_dir_str = CW2A(m_project_dir);
			//LoadImages(m_project_dir_str);
			m_pthread = AfxBeginThread(LoadImage_Thread, this);
			if (!m_pthread)
			{
				AfxMessageBox(L"加载图像线程创建失败！");
				GetDlgItem(IDC_SELECT_PROJECT)->EnableWindow(true);
				GetDlgItem(IDC_IS_COLOR)->EnableWindow(true);
				return;
			}
			GetDlgItem(IDC_IS_COLOR)->EnableWindow(false);
		}
		else
		{
			GetDlgItem(IDC_SELECT_PROJECT)->EnableWindow(true);
			GetDlgItem(IDC_IS_COLOR)->EnableWindow(true);
		}
	}
}

void CImgClassifyDlg::findFile(std::string path, std::vector<std::string>& res)
{
	res.clear();
	string p;
	intptr_t hFile = 0;                //文件句柄
	struct _finddata_t fileinfo;    //文件信息
	//查找文件夹中的第一个文件，然后利用句柄进行遍历
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != 0)
	{
		do
		{
			string path_s = p.assign(path).append("\\").append(fileinfo.name);
			if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
				continue;
			if (fileinfo.attrib == _A_SUBDIR)
			{
				//遍历子目录
				std::vector<std::string> temp;
				findFile(path_s, temp);
				for (size_t i = 0; i < temp.size(); i++)
					res.push_back(temp[i]);
				continue;
			}
			res.push_back(path_s);            //插入容器
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);    //关闭文件
	}
}

void CImgClassifyDlg::findFile(std::string path, std::vector<std::string>& res, std::vector<std::string>& _dirs)
{
	res.clear();
	_dirs.clear();
	string p;
	intptr_t hFile = 0;                //文件句柄
	struct _finddata_t fileinfo;    //文件信息
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != 0)
	{
		do
		{
			string path_s = p.assign(path).append("\\").append(fileinfo.name);
			if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
				continue;
			if (fileinfo.attrib == _A_SUBDIR)
			{
				_dirs.push_back(fileinfo.name);
			}
			res.push_back(path_s);            //插入容器
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);    //关闭文件
	}
}

int CImgClassifyDlg::PathDevision(std::string str, std::string & path, std::string & name)
{
	int i = str.find_last_of("/");
	int j = str.find_last_of("\\");
	int d = 0;
	int l = str.length();
	if (i < 0 && j < 0)
	{
		return -1;
	}
	d = MAX(i, j);
	path = str.substr(0, d + 1);
	name = str.substr(d + 1, l - d);
	return 0;
}

void CImgClassifyDlg::LoadImages(std::string fullpath)
{
	m_fullpath.clear();
	m_path.clear();
	m_name.clear();
	std::vector<std::string> temp;
	findFile(fullpath, temp, m_dirs);
	Mat src;
	string paths, names, types;

	CProgressCtrl myProCtrl;
	CRect rect, proRect;
	GetClientRect(&rect);
	proRect.left = rect.left + rect.Width() / 2 - 100;
	proRect.top = rect.top + rect.Height() / 2 - 20;
	proRect.right = rect.right - rect.Width() / 2 + 100;
	proRect.bottom = rect.bottom - rect.Height() / 2 + 20;
	//WS_CHILD|WS_VISIBLE|PBS_SMOOTHREVERSE
	myProCtrl.Create(WS_VISIBLE, proRect, this, 99); //创建位置、大小
	myProCtrl.SetRange(0, 100);

	for (size_t i = 0; i < temp.size(); i++)
	{
		int pos = (i + 1) * 100 / temp.size();
		src = imread(temp[i], m_is_color_flag);
		if (src.empty())
			continue;
		if (PathDevision(temp[i], paths, names) != 0)
			continue;
		this->m_fullpath.push_back(temp[i]);
		this->m_path.push_back(paths);
		this->m_name.push_back(names);
		myProCtrl.SetPos(pos);
	}
}

void CImgClassifyDlg::OnLbnSelchangeImgList()
{
	// TODO: 在此添加控件通知处理程序代码
	int n = m_unkown_list.GetCurSel();
	Mat src = imread(m_fullpath[n], m_is_color_flag);
	if(!src.empty())
		ShowMatImg(src, IDC_SHOW_IMG, _WIN_NAME_);
}

//删除
void CImgClassifyDlg::OnBnClickedDelClass()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pthread_del = AfxBeginThread(DelClass_Thread, this);
	if (!m_pthread_del)
	{
		AfxMessageBox(L"删除类型线程创建失败！");
		return;
	}
}

//确认
void CImgClassifyDlg::OnBnClickedButtonSure()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_COMBO_CLASS);
}


void CImgClassifyDlg::OnBnClickedButtonOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_BUTTON_OK);
}


void CImgClassifyDlg::OnBnClickedButtonNg()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_BUTTON_NG);
}

void CImgClassifyDlg::DlgItemShowEnale(bool flag)
{
	GetDlgItem(IDC_NEW_CLASS)->EnableWindow(flag);
	GetDlgItem(IDC_COMBO_CLASS)->EnableWindow(flag);
	GetDlgItem(IDC_BUTTON_SURE)->EnableWindow(flag);
	GetDlgItem(IDC_DEL_CLASS)->EnableWindow(flag);
	GetDlgItem(IDC_BUTTON_OK)->EnableWindow(flag);
	GetDlgItem(IDC_BUTTON_NG)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER1)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER2)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER3)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER4)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER5)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER6)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER7)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER8)->EnableWindow(flag);
	GetDlgItem(IDC_TRAIN_VAL_TEST)->EnableWindow(flag);
}

void CImgClassifyDlg::ThreadShowEnable(bool flag)
{
	GetDlgItem(IDC_NEW_CLASS)->EnableWindow(flag);
	GetDlgItem(IDC_COMBO_CLASS)->EnableWindow(flag);
	GetDlgItem(IDC_BUTTON_SURE)->EnableWindow(flag);
	GetDlgItem(IDC_DEL_CLASS)->EnableWindow(flag);
	GetDlgItem(IDC_BUTTON_OK)->EnableWindow(flag);
	GetDlgItem(IDC_BUTTON_NG)->EnableWindow(flag);
	GetDlgItem(IDC_SELECT_PROJECT)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER1)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER2)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER3)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER4)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER5)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER6)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER7)->EnableWindow(flag);
	GetDlgItem(IDC_ADDTO_OTHER8)->EnableWindow(flag);
	GetDlgItem(IDC_TRAIN_VAL_TEST)->EnableWindow(flag);
}

int CImgClassifyDlg::MoveMatToDir(cv::Mat src, std::string path, int index, bool show_flag)
{
	if (imwrite(path, src))
	{
		DirFile::_DeleteFile(m_fullpath[index]);
		m_fullpath.erase(m_fullpath.begin() + index);
		m_path.erase(m_path.begin() + index);
		m_name.erase(m_name.begin() + index);
		m_unkown_list.DeleteString(index);
		if (m_unkown_list.GetCount() > 0)
		{
			m_unkown_list.SetCurSel(0);
			if (show_flag)
			{
				Mat temp = imread(m_fullpath[0], m_is_color_flag);
				if (!temp.empty())
					ShowMatImg(temp, IDC_SHOW_IMG, _WIN_NAME_);
			}
		}
		else
			if(show_flag)
				ShowMatImg(init_mat, IDC_SHOW_IMG, _WIN_NAME_);
	}
	else
	{
		m_pathc.pop_back();
		m_namec.pop_back();
		AfxMessageBox(L"移动文件失败，请检查文件夹是否存在！");
		return -1;
	}
	return 0;
}

void CImgClassifyDlg::closeThread()
{
	if (m_open_thread_flag)
	{
		TerminateThread(m_pthread->m_hThread, m_pthread->m_nThreadID);
		m_open_thread_flag = false;
	}
	if (m_del_calass_flag)
	{
		TerminateThread(m_pthread_del->m_hThread, m_pthread_del->m_nThreadID);
		m_del_calass_flag = false;
	}
	if (m_train_val_test_falg)
	{
		TerminateThread(m_pthread_tvt->m_hThread, m_pthread_tvt->m_nThreadID);
		m_train_val_test_falg = false;
	}
}


BOOL CImgClassifyDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		HACCEL hAccel = m_hAccel;
		if (hAccel && ::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CImgClassifyDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		CString cstr;
		GetDlgItemText(IDC_STATIC_CNT, cstr);
		int n = m_unkown_list.GetCount();
		CString ccc; ccc.Format(L"%d", n);
		if (ccc != cstr)
			SetDlgItemText(IDC_STATIC_CNT, ccc);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CImgClassifyDlg::OnBnClickedButtonReturn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_namec.size() < 1 || m_pathc.size() < 1)
		return;
	string path = m_pathc.back();
	m_pathc.pop_back();
	string name = m_namec.back();
	m_namec.pop_back();
	string fullpath = DirFile::DirAddSubdir(path, name);
	Mat temp = imread(fullpath, m_is_color_flag);
	if (!temp.empty())
	{
		string newpath = DirFile::DirAddSubdir(m_project_dir_str, name);
		if (imwrite(newpath, temp))
		{
			DirFile::_DeleteFile(fullpath);
			m_unkown_list.InsertString(0, CString(name.c_str()));
			m_fullpath.insert(m_fullpath.begin(), newpath);
			m_path.insert(m_path.begin(), m_project_dir_str);
			m_name.insert(m_name.begin(), name);
			m_unkown_list.SetCurSel(0);
			ShowMatImg(temp, IDC_SHOW_IMG, _WIN_NAME_);
		}
	}
}

void CImgClassifyDlg::OnBnClickedButtonClear()
{
	m_pathc.clear();
	m_namec.clear();
}


void CImgClassifyDlg::OnSortMove(int dlgItemID)
{
	int index = m_unkown_list.GetCurSel();
	if (index < 0)
		return;
	CString cstr;
	GetDlgItemText(dlgItemID, cstr);
	string temp = CW2A(cstr);
	if (dlgItemID != IDC_COMBO_CLASS)
		temp = temp.substr(0, temp.length() - 3);
	if (temp == "None")
	{
		AfxMessageBox(L"这是一个空类型，请选择其他类型！");
		return;
	}
	temp = DirFile::DirAddSubdir(m_project_dir_str, temp);
	string fullpath = DirFile::DirAddSubdir(temp, m_name[index]);
	Mat src = imread(m_fullpath[index], m_is_color_flag);
	if (!src.empty())
	{
		m_pathc.push_back(temp);
		m_namec.push_back(m_name[index]);
		MoveMatToDir(src, fullpath, index);
	}
}


void CImgClassifyDlg::OnBnClickedAddtoOther1()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER1);
}


void CImgClassifyDlg::OnBnClickedAddtoOther2()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER2);
}


void CImgClassifyDlg::OnBnClickedAddtoOther3()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER3);
}


void CImgClassifyDlg::OnBnClickedAddtoOther4()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER4);
}


void CImgClassifyDlg::OnBnClickedAddtoOther5()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER5);
}


void CImgClassifyDlg::OnBnClickedAddtoOther6()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER6);
}


void CImgClassifyDlg::OnBnClickedAddtoOther7()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER7);
}


void CImgClassifyDlg::OnBnClickedAddtoOther8()
{
	// TODO: 在此添加控件通知处理程序代码
	OnSortMove(IDC_ADDTO_OTHER8);
}

void CImgClassifyDlg::Button_init()
{
	ifstream file;
	file.open(_PARAM_DATA_);
	if (!file.is_open())
	{
		AfxMessageBox(L"加载类型参数文件失败！");
		return;
	}
	int button_id[] = {
		IDC_ADDTO_OTHER1,
		IDC_ADDTO_OTHER2,
		IDC_ADDTO_OTHER3,
		IDC_ADDTO_OTHER4,
		IDC_ADDTO_OTHER5,
		IDC_ADDTO_OTHER6,
		IDC_ADDTO_OTHER7,
		IDC_ADDTO_OTHER8
	};
	for (int i = 0; i < _MAX_NUMS_; i++)
	{
		if (file.eof())
			break;
		char p[10];
		file.getline(p,10);
		if (p[0] == '\0')
			continue;
		sprintf_s(p, 10, "%s[%d]", p, i + 1);
		SetDlgItemText(button_id[i], CString(p));
	}
	file.close();
}


void CImgClassifyDlg::OnBnClickedTrainValTest()
{
	// TODO: 在此添加控件通知处理程序代码
	DirFile::CreateDir(DirFile::DirAddSubdir(m_project_dir_str,"train"));
	DirFile::CreateDir(DirFile::DirAddSubdir(m_project_dir_str, "val"));
	DirFile::CreateDir(DirFile::DirAddSubdir(m_project_dir_str, "test"));
	m_pthread_tvt = AfxBeginThread(TrainValTest_Thread, this);
	if (!m_pthread)
	{
		AfxMessageBox(L"训练类型划分线程创建失败！");
		return;
	}
}
