#pragma once
#include "../../giscommon/pubinc/gis_canvas.h"
#include "gismfc_export.h"
#include <agg_lib/pubinc/Engine.h>
#include <map>


// CGisCanvas
namespace GisCommon
{
using agg::Engine;

class GISMFC_EXPORT CGisCanvas;

class GISMFC_EXPORT CGisMouse : public gis_mouse
{
public:
	CGisCanvas* m_parent;

	bool is_left_down() const;
	bool is_middle_down() const;
	bool is_right_down() const;

	int getX() const;
	int getY() const;
};

class GISMFC_EXPORT CGisKeyboard : public gis_keyboard
{
public:
	CGisCanvas* m_parent;
	shift_state_t get_shift_state() const;
	bool pressed(int code) const;
};

class GISMFC_EXPORT CGisCanvas : public CWnd,public gis_canvas
{
	DECLARE_DYNAMIC(CGisCanvas)
public:
	typedef std::vector<agg::Engine> engines_t;

	typedef std::map<int,HCURSOR> cursors_t;

public:
	CToolTipCtrl m_tooltip;

	CGisCanvas();
	bool Create(CWnd* pParentWnd,const RECT& rc);
	virtual ~CGisCanvas();

	CGisMouse& get_mouse(){return m_mouse;}
	const CGisMouse& get_mouse()const{return m_mouse;}
	CGisKeyboard& get_keyboard(){return m_keyboard;}
	const CGisKeyboard& get_keyboard()const{return m_keyboard;}

	void InvalidateSurface(unsigned level);

	void resize(unsigned _width,unsigned _height);
	void set_info(gis_canvas_info info);

	static shift_state_t flags2shift_state(UINT nFlags);
	static shift_state_t get_shift_state();

	void set_engine_type(int val);
	unsigned get_engines_count() const{return engines.size();}
	void set_engines_count(unsigned val);
	void set_maximum_extent();
	agg::BaseEngine& get_engine(unsigned level);

	void do_draw(CDC& dc,unsigned invalidate_level);

    static double scroll_delta;
	static inline void enlarge_delta(geo::rect& mx)
	{
		mx.x1-=scroll_delta;
		mx.y1-=scroll_delta;
		mx.x2+=scroll_delta;
		mx.y2+=scroll_delta;
	}

    //Добавить курсор с определённым ID
	void add_cursor(int code,HCURSOR val);

    //Показывать или нет скролбары
	void show_scroll_bars(bool val);
	inline bool is_show_scroll_bars() const{return m_show_scroll_bars;}

    //Нотификация на OnContextMenu()
	notify<void (*)(CGisCanvas& canvas,int x,int y)> on_context_menu;

	static void RegisterClass();
protected:
	DECLARE_MESSAGE_MAP()

	engines_t engines;
	bool window_created;
	unsigned pended_invalidate_level;
	bool m_show_scroll_bars;

	CGisMouse m_mouse;
	CGisKeyboard m_keyboard;

	void update_cursor_state();
private:
	bool need_delete;

	bool m_scroll_event_mode;
	static const int max_scroll=0x4000;

	static const int hint_timer_id=1;
	int hintX;
	int hintY;
	std::string last_hint;
	int hint_delay;
	bool hint_visible;

	void init();
	void set_scroll_bars();
	void create_engines(int eng_type,unsigned surface_count);
	cursors_t cursors;

	void init_cursors();

	void hint_init();
	void hint_create();
	void hint_mouse_move_process();
	void hint_timer_process();
	bool hint_can_be_displayed();
	void hint_show(bool val);

	static LRESULT CALLBACK iWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR,LRESULT *pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg LRESULT OnPrintClient(WPARAM wParam, LPARAM lParam);
protected:
	virtual void PostNcDestroy();
};

}//namespace GisCommon
