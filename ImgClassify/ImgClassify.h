
// ImgClassify.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CImgClassifyApp: 
// �йش����ʵ�֣������ ImgClassify.cpp
//

class CImgClassifyApp : public CWinApp
{
public:
	CImgClassifyApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CImgClassifyApp theApp;