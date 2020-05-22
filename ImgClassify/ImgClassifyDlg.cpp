
// ImgClassifyDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CImgClassifyDlg �Ի���

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
	myProCtrl.Create(WS_VISIBLE, proRect, pWnd, 99); //����λ�á���С
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
	pWnd->SetDlgItemText(IDC_SELECT_PROJECT, L"�رչ���");
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
	myProCtrl.Create(WS_VISIBLE, proRect, pWnd, 99); //����λ�á���С
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
	myProCtrl.Create(WS_VISIBLE, proRect, pWnd, 99); //����λ�á���С
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


// CImgClassifyDlg ��Ϣ�������

BOOL CImgClassifyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_MAXIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	__init__();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CImgClassifyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CImgClassifyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CImgClassifyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	for (int i = 1000; i < 1040; i++)//��Ϊ�Ƕ���ؼ���������������ѭ��
	{
		CWnd *pWnd = GetDlgItem(i);
		if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())  //�ж��Ƿ�Ϊ�գ���Ϊ�Ի��򴴽�ʱ����ô˺���������ʱ�ؼ���δ����
		{
			CRect rect;   //��ȡ�ؼ��仯ǰ�Ĵ�С 
			pWnd->GetWindowRect(&rect);
			ScreenToClient(&rect);//���ؼ���Сת��Ϊ�ڶԻ����е���������
			rect.left = rect.left*cx / m_wndRect.Width();//�����ؼ���С
			rect.right = rect.right*cx / m_wndRect.Width();
			rect.top = rect.top*cy / m_wndRect.Height();
			rect.bottom = rect.bottom*cy / m_wndRect.Height();
			pWnd->MoveWindow(rect);//���ÿؼ���С 
		}
	}
	//���»�ô��ڳߴ�
	GetClientRect(&m_wndRect);
}


void CImgClassifyDlg::OnClose()
{
	// TODO: m�ڴ������Ϣ�����������/�����Ĭ��ֵ
	closeThread();

	CDialogEx::OnClose();
}


void CImgClassifyDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	closeThread();

	CDialogEx::OnOK();
}


void CImgClassifyDlg::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���
	closeThread();

	CDialogEx::OnCancel();
}

void CImgClassifyDlg::__init__()
{
	SetImgWindow(IDC_SHOW_IMG, _WIN_NAME_);
	//��ô��ڳߴ�
	GetClientRect(&m_wndRect);
	init_mat = Mat::Mat(500, 500, CV_8UC1, cvScalarAll(0xff));

	Button_init();
	srand(int(time(0)));
	DlgItemShowEnale(false);

	ShowMatImg(init_mat, IDC_SHOW_IMG, _WIN_NAME_);
	m_hAccel = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	SetTimer(1, 1000, NULL);
}

//��ʼ���ã�ȷ�����ڿؼ��ʹ�������
void CImgClassifyDlg::SetImgWindow(int Dlgitem, const char* str)
{
	namedWindow(str, WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle(str);
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(Dlgitem)->m_hWnd);  //picture�ؼ�
	::ShowWindow(hParent, SW_HIDE);
}

//��ʾͼ��
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

//�½�
void CImgClassifyDlg::OnBnClickedNewClass()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	sInfo.lpszTitle = _T("��ѡ��ͼ��·��");
	sInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_DONTGOBELOWDOMAIN;
	sInfo.lpfn = NULL;

	// ��ʾ�ļ���ѡ��Ի���  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// ȡ���ļ�����  
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

//��/�ر�
void CImgClassifyDlg::OnBnClickedSelectProject()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		SetDlgItemText(IDC_SELECT_PROJECT, L"�򿪹���");
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
				AfxMessageBox(L"����ͼ���̴߳���ʧ�ܣ�");
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
	intptr_t hFile = 0;                //�ļ����
	struct _finddata_t fileinfo;    //�ļ���Ϣ
	//�����ļ����еĵ�һ���ļ���Ȼ�����þ�����б���
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != 0)
	{
		do
		{
			string path_s = p.assign(path).append("\\").append(fileinfo.name);
			if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
				continue;
			if (fileinfo.attrib == _A_SUBDIR)
			{
				//������Ŀ¼
				std::vector<std::string> temp;
				findFile(path_s, temp);
				for (size_t i = 0; i < temp.size(); i++)
					res.push_back(temp[i]);
				continue;
			}
			res.push_back(path_s);            //��������
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);    //�ر��ļ�
	}
}

void CImgClassifyDlg::findFile(std::string path, std::vector<std::string>& res, std::vector<std::string>& _dirs)
{
	res.clear();
	_dirs.clear();
	string p;
	intptr_t hFile = 0;                //�ļ����
	struct _finddata_t fileinfo;    //�ļ���Ϣ
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
			res.push_back(path_s);            //��������
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);    //�ر��ļ�
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
	myProCtrl.Create(WS_VISIBLE, proRect, this, 99); //����λ�á���С
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int n = m_unkown_list.GetCurSel();
	Mat src = imread(m_fullpath[n], m_is_color_flag);
	if(!src.empty())
		ShowMatImg(src, IDC_SHOW_IMG, _WIN_NAME_);
}

//ɾ��
void CImgClassifyDlg::OnBnClickedDelClass()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_pthread_del = AfxBeginThread(DelClass_Thread, this);
	if (!m_pthread_del)
	{
		AfxMessageBox(L"ɾ�������̴߳���ʧ�ܣ�");
		return;
	}
}

//ȷ��
void CImgClassifyDlg::OnBnClickedButtonSure()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_COMBO_CLASS);
}


void CImgClassifyDlg::OnBnClickedButtonOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_BUTTON_OK);
}


void CImgClassifyDlg::OnBnClickedButtonNg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		AfxMessageBox(L"�ƶ��ļ�ʧ�ܣ������ļ����Ƿ���ڣ�");
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
	// TODO: �ڴ����ר�ô����/����û���
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		AfxMessageBox(L"����һ�������ͣ���ѡ���������ͣ�");
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER1);
}


void CImgClassifyDlg::OnBnClickedAddtoOther2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER2);
}


void CImgClassifyDlg::OnBnClickedAddtoOther3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER3);
}


void CImgClassifyDlg::OnBnClickedAddtoOther4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER4);
}


void CImgClassifyDlg::OnBnClickedAddtoOther5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER5);
}


void CImgClassifyDlg::OnBnClickedAddtoOther6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER6);
}


void CImgClassifyDlg::OnBnClickedAddtoOther7()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER7);
}


void CImgClassifyDlg::OnBnClickedAddtoOther8()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnSortMove(IDC_ADDTO_OTHER8);
}

void CImgClassifyDlg::Button_init()
{
	ifstream file;
	file.open(_PARAM_DATA_);
	if (!file.is_open())
	{
		AfxMessageBox(L"�������Ͳ����ļ�ʧ�ܣ�");
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DirFile::CreateDir(DirFile::DirAddSubdir(m_project_dir_str,"train"));
	DirFile::CreateDir(DirFile::DirAddSubdir(m_project_dir_str, "val"));
	DirFile::CreateDir(DirFile::DirAddSubdir(m_project_dir_str, "test"));
	m_pthread_tvt = AfxBeginThread(TrainValTest_Thread, this);
	if (!m_pthread)
	{
		AfxMessageBox(L"ѵ�����ͻ����̴߳���ʧ�ܣ�");
		return;
	}
}
