#pragma once
#include "../algo/field.h"
#include <notification/notification.hpp>


// CMfcField

class CMfcField : public CWnd,public Gomoku::field_t
{
	DECLARE_DYNAMIC(CMfcField)
protected:
	CFont m_font;
	CBitmap bEmpty;
	CBitmap bKrestik;
	CBitmap bNolik;
	CBitmap bKrestikLast;
	CBitmap bNolikLast;
	Gomoku::point center;
	int width;
	int height;
	static const int max_scroll=0x4000;

	
public:
	static const int box_size=18;
	bool show_move_numbers;

	CMfcField();
	virtual ~CMfcField();


	CPoint world2pix(const Gomoku::point& pt) const;
	Gomoku::point pix2world(const CPoint& pt) const;
	void set_scroll_bars();
	CPoint mouse_pos() const;

	Notification::notify<void (*)(Gomoku::point pos)> OnLMouseDown;
	Notification::notify<void (*)(Gomoku::point pos)> OnLMouseUp;
	Notification::notify<void (*)(Gomoku::point pos)> on_mouse_move;
	Notification::notify<void (*)(CDC& dc)> on_after_paint;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


