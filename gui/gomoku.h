// gomoku.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CgomokuApp:
// See gomoku.cpp for the implementation of this class
//

class CgomokuApp : public CWinApp
{
    HACCEL m_hAccel;
public:
	CgomokuApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
    virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
};

extern CgomokuApp theApp;