// gomokuDlg.h : header file
//

#pragma once
#include "mfc_field.h"
#include "DlgResizeHelper.h"
#include "../algo/game.h"
#include "afxwin.h"
#include "../algo/wsplayer.h"
#include "ThreadProcessor.h"
#include <boost/signals/connection.hpp>

class mfcPlayer : public Gomoku::iplayer_t
{
	CMfcField* fd;
	CBitmap bmp;
	bool inited;

	boost::signals::scoped_connection hld_mouse_down;
	boost::signals::scoped_connection hld_mouse_up;
	boost::signals::scoped_connection hld_mouse_move;
	boost::signals::scoped_connection hld_after_draw;

	void fieldLMouseDown(Gomoku::point pos);
	void fieldLMouseUp(Gomoku::point pos);
	void fieldMouseMove(Gomoku::point pos);
	void afterDraw(CDC& dc);
public:
	mfcPlayer(){fd=0;inited=false;}
	mfcPlayer(const mfcPlayer& rhs){fd=0;}
	
	void init(Gomoku::game_t& _gm,Gomoku::Step _cl);
	void delegate_step();

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
    }

	POLIMVAR_IMPLEMENT_CLONE(mfcPlayer )
};

class ThreadPlayer : public Gomoku::iplayer_t
{
public:
private:
	CThreadProcessor processor;
	Gomoku::player_ptr pl;
	Gomoku::player_ptr null_player;
	Gomoku::game_t mirror_gm;
	Gomoku::step_t last_step;

	boost::signals::scoped_connection hld_execute;
	boost::signals::scoped_connection hld_complete;
	boost::signals::scoped_connection hld_errors;

	void MirrorExecute();
	void TaskComplete();
	void TaskErrors(const CThreadProcessor::errors_t& vals);

	void clear();
public:
	ThreadPlayer(){}
	~ThreadPlayer(){clear();}
	ThreadPlayer(const Gomoku::player_ptr& _pl){pl=_pl;}
	ThreadPlayer(const ThreadPlayer& rhs){if(rhs.pl)pl=Gomoku::player_ptr(rhs.pl->clone());}
	
	void init(Gomoku::game_t& _gm,Gomoku::Step _cl);
	void delegate_step();
	Gomoku::player_ptr get_player() const{return pl;}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
        ar & BOOST_SERIALIZATION_NVP(pl);
    }

	POLIMVAR_IMPLEMENT_CLONE(ThreadPlayer)
};


class NullPlayer : public Gomoku::iplayer_t
{
public:
	NullPlayer(){}
	
	void init(Gomoku::game_t& _gm,Gomoku::Step _cl){}
	void delegate_step(){}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iplayer_t);
    }

	POLIMVAR_IMPLEMENT_CLONE(NullPlayer)
};

// CgomokuDlg dialog
class CgomokuDlg : public CDialog
{
// Construction
public:
	typedef boost::shared_ptr<CMfcField> field_ptr;

	CgomokuDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GOMOKU_DIALOG };

private:
	field_ptr m_field;
	DlgResizeHelper szr;
	Gomoku::game_t game;
	boost::signals::scoped_connection hld_step;
	
	
	void gameNextStep(const Gomoku::game_t&);
	void start_game();
	void check_state();
	Gomoku::player_ptr create_player(const CComboBox& cb,Gomoku::Step st);
	int player2index(Gomoku::iplayer_t& pl);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedStart();
	CButton mStop;
	CComboBox mPlayer1;
	CComboBox mPlayer2;
	CButton m_Start;
	afx_msg void OnBnClickedStop();
	afx_msg void OnCbnSelchangePlayer1();
	afx_msg void OnCbnSelchangePlayer2();
	afx_msg void OnLoadGame();
	afx_msg void OnSaveGame();
	afx_msg void OnSaveStringField();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditShowmovenumber(CCmdUI *pCmdUI);
	afx_msg void OnEditShowmovenumber();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnClose();
};
