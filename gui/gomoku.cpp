// gomoku.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "gomoku.h"
#include "gomokuDlg.h"
#include <boost/filesystem/path.hpp>
#include "../extern/object_progress.hpp"
#include <boost/filesystem/operations.hpp>


// CgomokuApp

BEGIN_MESSAGE_MAP(CgomokuApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CgomokuApp construction

CgomokuApp::CgomokuApp()
{
    m_hAccel = NULL;
}


// The one and only CgomokuApp object

CgomokuApp theApp;


//ObjectProgress::log_to_file lf;

// CgomokuApp initialization

BOOL CgomokuApp::InitInstance()
{
	CWinApp::InitInstance();

    m_hAccel=LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

    CgomokuDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


BOOL CgomokuApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
    if(m_hAccel)
    {
        if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_hAccel, lpMsg)) 
            return TRUE;
    }
	
    return CWinApp::ProcessMessageFilter(code, lpMsg);
}
