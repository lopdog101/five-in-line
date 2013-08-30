// gomokuDlg.h : header file
//

#pragma once
#include "DlgResizeHelper.h"
#include "afxwin.h"
#include "InterractivePlayers.h"


// CgomokuDlg dialog
class CgomokuDlg : public CDialog
{
// Construction
public:
	typedef boost::shared_ptr<CMfcField> field_ptr;

    static const DWORD WM_CHECK_STATE=WM_USER+1;

	CgomokuDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GOMOKU_DIALOG };

private:
	field_ptr m_field;
	DlgResizeHelper szr;
	Gomoku::game_t game;
    Gomoku::steps_t redo_steps;
	boost::signals::scoped_connection hld_step;

    ObjectProgress::logout_debug log_dbg;
    ObjectProgress::logout_file log_file;
	
	
	void gameNextStep(const Gomoku::iplayer_t& pl,const Gomoku::point& pt);
	void invalidate_field_check_state();
    void check_state();
    void enable_button(int ButtonId,bool val);

	Gomoku::player_ptr create_player(const CComboBox& cb,Gomoku::Step st);
	int player2index(Gomoku::iplayer_t& pl);

    void start();
    void pause();
    void restart_if_next_player_human();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    CMFCToolBar       m_wndToolBar;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox mPlayer1;
	CComboBox mPlayer2;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCbnSelchangePlayer1();
	afx_msg void OnCbnSelchangePlayer2();
	afx_msg void OnLoadGame();
	afx_msg void OnSaveGame();
	afx_msg void OnLoadStringField();
	afx_msg void OnSaveStringField();
	afx_msg void OnUpdateEditShowmovenumber(CCmdUI *pCmdUI);
	afx_msg void OnEditShowmovenumber();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg LRESULT OnPostCheck(WPARAM, LPARAM);
	afx_msg void OnTapeStart();
	afx_msg void OnTapeRewind();
	afx_msg void OnTapePlay();
	afx_msg void OnTapeForward();
	afx_msg void OnTapeEnd();
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnClose();
};
