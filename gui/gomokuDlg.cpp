// gomokuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gomoku.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

#include "gomokuDlg.h"
#include ".\gomokudlg.h"



#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
namespace fs=boost::filesystem;

#include "../algo/check_player.h"
#include "../algo/game_xml.h"
#include "../algo/env_variables.h"

BOOST_CLASS_EXPORT(Gomoku::check_player_t)
BOOST_CLASS_EXPORT(Gomoku::WsPlayer::wsplayer_t)
BOOST_CLASS_EXPORT(ThreadPlayer)
BOOST_CLASS_EXPORT(mfcPlayer)
BOOST_CLASS_EXPORT(NullPlayer)


// CgomokuDlg dialog

CgomokuDlg::CgomokuDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CgomokuDlg::IDD, pParent),m_field(new CMfcField)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    log_dbg.open();
    
    log_file.file_name="five_in_line.log";
    log_file.print_timestamp=true;
    log_file.open();

	game.fieldp=m_field;
}

void CgomokuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STOP, mStop);
	DDX_Control(pDX, IDC_PLAYER1, mPlayer1);
	DDX_Control(pDX, IDC_PLAYER2, mPlayer2);
	DDX_Control(pDX, IDC_START, m_Start);
}

BEGIN_MESSAGE_MAP(CgomokuDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_START, OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
	ON_CBN_SELCHANGE(IDC_PLAYER1, OnCbnSelchangePlayer1)
	ON_CBN_SELCHANGE(IDC_PLAYER2, OnCbnSelchangePlayer2)
	ON_COMMAND(ID_OPERATION_LOADGAME, OnLoadGame)
	ON_COMMAND(ID_OPERATION_SAVEGAME, OnSaveGame)
	ON_COMMAND(ID_OPERATION_SAVESTRINGFIELD, OnSaveStringField)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SHOWMOVENUMBER, OnUpdateEditShowmovenumber)
	ON_COMMAND(ID_EDIT_SHOWMOVENUMBER, OnEditShowmovenumber)
	ON_WM_INITMENUPOPUP()
	ON_WM_CLOSE()
	ON_COMMAND(ID_TAPE_START, OnTapeStart)
END_MESSAGE_MAP()


// CgomokuDlg message handlers

BOOL CgomokuDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    Gomoku::scan_enviropment_variables();

	ObjectProgress::log_generator lg(true);
    Gomoku::print_used_enviropment_variables(lg);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CRect rc;
	GetDlgItem(IDC_FIELD_PLACE)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_field->Create(0,"",WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_BORDER,rc,this,IDC_FIELD);
    
	if (!m_wndToolBar.CreateEx(this,TBSTYLE_FLAT | TBSTYLE_TRANSPARENT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_HIDE_INPLACE)
        ||!m_wndToolBar.LoadToolBar(IDR_TOOLBAR1,IDB_TB,0,0,IDB_TB_DIS,0,IDB_TB))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    m_wndToolBar.SetPaneStyle( m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_ANY) );
    CSize sz = m_wndToolBar.CalcFixedLayout( FALSE, TRUE );
    m_wndToolBar.SetWindowPos( NULL, 0, 0, sz.cx+2, sz.cy,SWP_NOACTIVATE | SWP_NOZORDER );


    szr.Init(m_hWnd);
	szr.Fix(m_field->m_hWnd,szr.kLeftRight,szr.kTopBottom);
	szr.Fix(IDC_START,szr.kWidthRight,szr.kHeightTop);
	szr.Fix(IDC_STOP,szr.kWidthRight,szr.kHeightTop);
	szr.Fix(IDC_PLAYER1,szr.kWidthRight,szr.kHeightTop);
	szr.Fix(IDC_PLAYER1_LABEL,szr.kWidthRight,szr.kHeightTop);
	szr.Fix(IDC_PLAYER2,szr.kWidthRight,szr.kHeightTop);
	szr.Fix(IDC_PLAYER2_LABEL,szr.kWidthRight,szr.kHeightTop);

	mPlayer1.SetCurSel(2);game.set_krestik(create_player(mPlayer1,Gomoku::st_krestik));
	mPlayer2.SetCurSel(0);game.set_nolik(create_player(mPlayer2,Gomoku::st_nolik));
	check_state();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CgomokuDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CgomokuDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CgomokuDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	szr.OnSize();
}

void CgomokuDlg::OnBnClickedStart()
{
	start_game();
	check_state();
}

void CgomokuDlg::gameNextStep(const Gomoku::game_t&)
{
	check_state();
	m_field->set_scroll_bars();
	m_field->Invalidate();
	m_field->UpdateWindow();
	if(!game.is_game_over())return;
	hld_step.disconnect();
	if(game.field().size()%2)MessageBox("krestik win",MB_OK);
	else MessageBox("nolik win",MB_OK);
}

void CgomokuDlg::start_game()
{
	hld_step=game.OnNextStep.connect(boost::bind(&CgomokuDlg::gameNextStep,this,_1) );
	game.begin_play();
	m_field->Invalidate();
}

Gomoku::player_ptr CgomokuDlg::create_player(const CComboBox& cb,Gomoku::Step st)
{
	Gomoku::player_ptr ret;
	switch(cb.GetCurSel())
	{
	case 0:ret=Gomoku::player_ptr(new mfcPlayer);break;
	default:
	{
		Gomoku::player_ptr sub(new Gomoku::WsPlayer::wsplayer_t);
		ret=Gomoku::player_ptr(new ThreadPlayer(sub));
		break;
	}

	}
	return ret;
}

int CgomokuDlg::player2index(Gomoku::iplayer_t& pl)
{
	if(dynamic_cast<mfcPlayer*>(&pl)!=0)return 0;

	ThreadPlayer* tpl=dynamic_cast<ThreadPlayer*>(&pl);
	if(!tpl)return -1;

	Gomoku::player_ptr sub=tpl->get_player();
	if(!sub)return -1;

	if(dynamic_cast<Gomoku::WsPlayer::wsplayer_t*>(&*sub)!=0) return 1;
	return -1;
}


void CgomokuDlg::check_state()
{
	bool started=game.is_play();
	mStop.EnableWindow(started);
	mPlayer1.SetCurSel(player2index(*game.get_krestik()));
	mPlayer2.SetCurSel(player2index(*game.get_nolik()));

    bool isStartEnabled=!started&&mPlayer1.GetCurSel()!=-1&&mPlayer2.GetCurSel()!=-1;
	m_Start.EnableWindow(isStartEnabled);
}

void CgomokuDlg::OnBnClickedStop()
{
	game.end_play();
	m_field->Invalidate();
	check_state();
}

void CgomokuDlg::OnCbnSelchangePlayer1()
{
	game.set_krestik(create_player(mPlayer1,Gomoku::st_krestik));
	check_state();
	if(game.is_play())game.continue_play();
}

void CgomokuDlg::OnCbnSelchangePlayer2()
{
	game.set_nolik(create_player(mPlayer2,Gomoku::st_nolik));
	check_state();
	if(game.is_play())game.continue_play();
}

void CgomokuDlg::OnLoadGame()
{
    hld_step.disconnect();

    try
    {
	    CFileDialog dlg(TRUE,0,0,OFN_FILEMUSTEXIST,"Games (*.gm)|*.gm||");
	    if(dlg.DoModal()!=IDOK)return;

    //    Xpat::load_from_xml_file(,game);
        std::ifstream ifs;
        ifs.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        ifs.open(dlg.GetPathName().GetString());
        boost::archive::xml_iarchive ia(ifs);

        ia >> BOOST_SERIALIZATION_NVP(game);

        m_field->Invalidate();
        m_field->UpdateWindow();
        check_state();
        if(game.is_play())
        {
            hld_step=game.OnNextStep.connect(boost::bind(&CgomokuDlg::gameNextStep,this,_1) );
	        game.continue_play();
        }
    }
    catch(std::exception& e)
    {
        AfxMessageBox(e.what());
    }

}

void CgomokuDlg::OnSaveGame()
{
	CFileDialog dlg(FALSE,".gm",0,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Games (*.gm)|*.gm||");
	if(dlg.DoModal()!=IDOK)return;

    std::ofstream ofs;

    ofs.open(dlg.GetPathName().GetString());
    ofs.exceptions( std::ifstream::failbit | std::ifstream::badbit );

    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(game);
}

void CgomokuDlg::OnSaveStringField()
{
	CFileDialog dlg(FALSE,".txt",0,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Strings (*.txt)|*.txt||");
	if(dlg.DoModal()!=IDOK)return;

	Gomoku::steps_t steps=game.field().get_steps();
	std::string str=print_steps(steps);

    std::ofstream ofs;

    ofs.open(dlg.GetPathName().GetString());
    ofs.exceptions( std::ifstream::failbit | std::ifstream::badbit );
    ofs<<str;
}



void CgomokuDlg::OnEditUndo()
{
	if(game.field().get_steps().size()<2)return;
	bool game_over=game.is_game_over();
	Gomoku::steps_t st=game.field().get_steps();
	st.resize(st.size()-2);
	game.field().set_steps(st);
	m_field->Invalidate();
	m_field->UpdateWindow();
	check_state();
	if(game_over)
	{
		hld_step=game.OnNextStep.connect(boost::bind(&CgomokuDlg::gameNextStep,this,_1) );
		game.continue_play();
	}
}

void CgomokuDlg::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(game.field().get_steps().size()>=2);
}

void CgomokuDlg::OnUpdateEditShowmovenumber(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_field->show_move_numbers? 1:0);
}

void CgomokuDlg::OnEditShowmovenumber()
{
	m_field->show_move_numbers=!m_field->show_move_numbers;
	m_field->Invalidate();
}

void CgomokuDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    CCmdUI state;
    state.m_pMenu=pPopupMenu;

	HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu==pPopupMenu->m_hMenu)state.m_pParentMenu = pPopupMenu;
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
           // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
           (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
           int nIndexMax = ::GetMenuItemCount(hParentMenu);
           for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
           {
            if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
            {
                // When popup is found, m_pParentMenu is containing menu.
                state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                break;
            }
           }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
      state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
           continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
           // Possibly a popup menu, route to first item of that popup.
           state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
           if (state.m_pSubMenu == NULL ||
            (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
            state.m_nID == (UINT)-1)
           {
            continue;       // First item of popup can't be routed to.
           }
           state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
           // Normal menu item.
           // Auto enable/disable if frame window has m_bAutoMenuEnable
           // set and command is _not_ a system command.
           state.m_pSubMenu = NULL;
           state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
           state.m_nIndex -= (state.m_nIndexMax - nCount);
           while (state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
           {
            state.m_nIndex++;
           }
        }
        state.m_nIndexMax = nCount;
    }
}

void CgomokuDlg::OnOK()
{
}

void CgomokuDlg::OnCancel()
{
}

void CgomokuDlg::OnClose()
{
	EndDialog(IDCLOSE);
}

void CgomokuDlg::OnTapeStart()
{
    OnBnClickedStart();
}
