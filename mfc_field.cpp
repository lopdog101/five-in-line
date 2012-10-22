#include "stdafx.h"
#include "gomoku.h"
#include "mfc_field.h"
#include ".\mfc_field.h"


// CMfcField

IMPLEMENT_DYNAMIC(CMfcField, CWnd)
CMfcField::CMfcField() : center(0,0)
{
	width=0;
	height=0;
	show_move_numbers=true;

	m_font.CreateFont(
			11,0,0,0,
			FW_BOLD,FALSE,FALSE,0,
			ANSI_CHARSET,              // nCharSet
			OUT_TT_ONLY_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			CLEARTYPE_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			"Arial");                 // lpszFacename
}

CMfcField::~CMfcField()
{
}


BEGIN_MESSAGE_MAP(CMfcField, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CMfcField message handlers


void CMfcField::OnPaint()
{
	CPaintDC window_dc(this);
	CRect rc;
	GetClientRect(&rc);

	CBitmap b;
	b.CreateCompatibleBitmap(&window_dc,rc.Width(),rc.Height());
	CDC dc;
	dc.CreateCompatibleDC(&window_dc);
	dc.SelectObject(&b);

	CBrush brh;
	brh.CreateSolidBrush(RGB(255,255,255));
	dc.FillRect(&rc,&brh);

	CDC bmp_dc;
	bmp_dc.CreateCompatibleDC(&dc);

	dc.SetTextColor(RGB(255,255,255));
	dc.SetBkMode(TRANSPARENT);
	CFont* def_font=dc.SelectObject(&m_font);

	for(int y=0;y<height;y+=box_size)
	for(int x=0;x<width;x+=box_size)
	{
		Gomoku::point pos=pix2world(CPoint(x,y));
		CBitmap* bmp;
		bool last=false;
		const Gomoku::steps_t& steps=get_steps();

		if(!empty()&&pos==back()||size()>=2&&pos==steps[size()-2])last=true;

		Gomoku::Step step=at(pos);

		switch(step)
		{
		case Gomoku::st_krestik:bmp=last? &bKrestikLast:&bKrestik;break;
		case Gomoku::st_nolik:bmp=last? &bNolikLast:&bNolik;break;
		default:bmp=&bEmpty;
		}

		bmp_dc.SelectObject(bmp);
		dc.BitBlt(x,y,box_size,box_size,&bmp_dc,0,0,SRCCOPY);

		if(show_move_numbers&&step!=Gomoku::st_empty)
		{
			Gomoku::steps_t::const_iterator it=std::find(steps.begin(),steps.end(),Gomoku::step_t(step,pos.x,pos.y));
			unsigned move_num=1+(it-steps.begin());

			CString smove_num;
			smove_num.Format("%u",move_num);
			CSize sz=dc.GetTextExtent(smove_num);
			dc.TextOut(x+(box_size-sz.cx)/2,y+(box_size-sz.cy)/2,smove_num);
		}
	}
	on_after_paint(dc);

	dc.SelectObject(def_font);
	window_dc.BitBlt(0,0,rc.Width(),rc.Height(),&dc,0,0,SRCCOPY);
}

CPoint CMfcField::world2pix(const Gomoku::point& pt) const
{
	CPoint ret((pt.x+center.x+width/(box_size*2))*box_size,(pt.y+center.y+height/(box_size*2))*box_size);
	return ret;
}

Gomoku::point CMfcField::pix2world(const CPoint& pt) const
{
	Gomoku::point ret(pt.x/box_size-center.x-width/(box_size*2),pt.y/box_size-center.y-height/(box_size*2));
	return ret;
}

int CMfcField::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	bEmpty.LoadBitmap(IDB_EMPTY);
	bKrestik.LoadBitmap(IDB_KRESTIK);
	bNolik.LoadBitmap(IDB_NOLIK);
	bKrestikLast.LoadBitmap(IDB_KRESTIK_LAST);
	bNolikLast.LoadBitmap(IDB_NOLIK_LAST);

	ShowScrollBar(SB_BOTH,true);
	set_scroll_bars();

	return 0;
}

void CMfcField::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);
	OnLMouseDown(pix2world(point));
}

void CMfcField::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);
	OnLMouseUp(pix2world(point));
}

void CMfcField::set_scroll_bars()
{
	SCROLLINFO si;
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=SIF_DISABLENOSCROLL|SIF_PAGE|SIF_POS|SIF_RANGE;
	si.nMin=0;
	si.nMax=max_scroll-1;
	si.nPage=0;
	si.nPos=0;

	Gomoku::rect mx=get_bound();

	CRect crc;
	GetClientRect(&crc);

	Gomoku::rect rc(-crc.Width()/(box_size*2),-crc.Height()/(box_size*2),crc.Width()/(box_size*2),crc.Height()/(box_size*2));
	rc.x1-=center.x;rc.x2-=center.x;
	rc.y1-=center.y;rc.y2-=center.y;

	if(mx.width()==0)
	{
		si.nPage=max_scroll;
		si.nPos=0;
	}
	else
	{
		si.nPage=static_cast<UINT>( max_scroll*rc.width()/mx.width() );
		if(rc.x1<mx.x1)si.nPos=0;
		else si.nPos=static_cast<int>( max_scroll*(rc.x1-mx.x1)/mx.width() );
	}
	SetScrollInfo(SB_HORZ,&si,true);

	if(mx.height()==0)
	{
		si.nPage=max_scroll;
		si.nPos=0;
	}
	else
	{
		si.nPage=static_cast<UINT>( max_scroll*rc.height()/mx.height() );
		if(rc.y1<mx.y1)si.nPos=0;
		else si.nPos=static_cast<int>( max_scroll*(rc.y1-mx.y1)/mx.height() );
	}
	SetScrollInfo(SB_VERT,&si,true);
}

void CMfcField::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	width=cx;
	height=cy;
	set_scroll_bars();
}

void CMfcField::OnMouseMove(UINT nFlags, CPoint point)
{
	on_mouse_move(pix2world(point));
	CWnd::OnMouseMove(nFlags, point);
}

CPoint CMfcField::mouse_pos() const
{
	CPoint pt;
	if(!GetCursorPos(&pt)) return CPoint(0,0);
	ScreenToClient(&pt);
	return pt;
}
