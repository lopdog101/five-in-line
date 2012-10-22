// gomoku.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "gomoku.h"
#include "gomokuDlg.h"
#include <boost/filesystem/path.hpp>
#include <object_progress/log_to_file.hpp>
#include <object_progress/perfomance.hpp>
#include <boost/filesystem/operations.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CgomokuApp

BEGIN_MESSAGE_MAP(CgomokuApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CgomokuApp construction

CgomokuApp::CgomokuApp()
{
	boost::filesystem::path::default_name_check(boost::filesystem::native);
}


// The one and only CgomokuApp object

CgomokuApp theApp;


ObjectProgress::log_to_file lf;

// CgomokuApp initialization

BOOL CgomokuApp::InitInstance()
{
	CWinApp::InitInstance();

	lf.file_name="five_in_line.log";
	lf.create_empty=true;
	lf.set_manager(ObjectProgress::log_manager_singleton::instance());
	lf.open();
	boost::filesystem::initial_path();

	ObjectProgress::perfomance::precision=ObjectProgress::perfomance::pr_microsec;


	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

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
