// src\GisCanvas.cpp : implementation file
//

#include "stdafx.h"
#include "..\pubinc\mfcCanvas.h"
#include "../pubinc/mfcException.h"
#include "../../giscommon/pubinc/giscommon_exception.h"
#include "../gismfc_rc.h"
#include <limits>
#include <agg_lib/pubinc/agg_auto_draw.h>
#include <object_progress/object_progress.hpp>
#include "../pubinc/gismfc_exception_catch.h"

namespace GisCommon
{

namespace 
{
	gis_canvas* createCGisCanvas()
	{
		return new CGisCanvas();
	}
}

double CGisCanvas::scroll_delta=16;


// CGisCanvas

IMPLEMENT_DYNAMIC(CGisCanvas, CWnd)

CGisCanvas::CGisCanvas()
{
	need_delete=false;
	init();
	m_show_scroll_bars=true;
	need_flip_y=true;
	m_scroll_event_mode=false;
	window_created=false;
	pended_invalidate_level=0;
	create_engines(engine_type,engines_count);
	hint_init();
}

bool CGisCanvas::Create(CWnd* pParentWnd,const RECT& rc)
{
	if(!CWnd::Create(0,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP,rc,pParentWnd,0,0))return false;
	window_created=true;
	create_engines(engine_type,engines.size());
	
	width=rc.right-rc.left;
	height=rc.bottom-rc.top;

	show_scroll_bars(m_show_scroll_bars);

	init_cursors();
	hint_create();

	return true;
}


CGisCanvas::~CGisCanvas()
{
}

void CGisCanvas::init()
{
	m_mouse.m_parent=this;
	m_keyboard.m_parent=this;
}

void CGisCanvas::InvalidateSurface(unsigned level)
{
	level=std::min(level,engines.size()-1);
	if(level<pended_invalidate_level)return;
	pended_invalidate_level=level;
	set_scroll_bars();
	if(window_created)Invalidate(false);
}

shift_state_t CGisCanvas::flags2shift_state(UINT nFlags)
{
	return shift_state_t(
		(nFlags&MK_SHIFT)!=0,
		(GetKeyState(VK_MENU)&0x8000)!=0,
		(nFlags&MK_CONTROL)!=0,
		(nFlags&MK_LBUTTON)!=0,
		(nFlags&MK_MBUTTON)!=0,
		(nFlags&MK_RBUTTON)!=0 );
}

shift_state_t CGisCanvas::get_shift_state()
{
	return shift_state_t(
		(GetKeyState(VK_SHIFT)&0x8000)!=0,
		(GetKeyState(VK_MENU)&0x8000)!=0,
		(GetKeyState(VK_CONTROL)&0x8000)!=0,
		(GetKeyState(VK_LBUTTON)&0x8000)!=0,
		(GetKeyState(VK_MBUTTON)&0x8000)!=0,
		(GetKeyState(VK_RBUTTON)&0x8000)!=0 );
}


void CGisCanvas::resize(unsigned _width,unsigned _height)
{
	if(_width==0||_height==0)return;
	if(width==_width&&height==_height) return;
	gis_canvas::resize(_width,_height);
	RECT rc;
	RECT rccl;
	GetWindowRect(&rc);
	GetParent()->ScreenToClient(&rc);
	GetClientRect(&rccl);
	int dx=(rc.right-rc.left)-(rccl.right-rccl.left);
	int dy=(rc.bottom-rc.top)-(rccl.bottom-rccl.top);
	rc.right=rc.left+width+dx;
	rc.bottom=rc.top+height+dy;
	MoveWindow(&rc);
	set_scroll_bars();

	if(!window_created)return;

	for(unsigned i=0;i<engines.size();i++)
		engines[i]->Resize(width,height,m_hWnd);
	if(auto_redraw) InvalidateAll();
}

// Внешнее изменение экрана
void CGisCanvas::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if(cx==0||cy==0)return;
	if(cx==width&&cy==height) return;
	CRect tool_rect;
	GetClientRect(&tool_rect);
	m_tooltip.SetToolRect(this,1,&tool_rect);
	gis_canvas::resize(cx,cy);
	set_scroll_bars();

	for(unsigned i=0;i<engines.size();i++)
		engines[i]->Resize(width,height,m_hWnd);
	if(auto_redraw) InvalidateAll();
}

void CGisCanvas::show_scroll_bars(bool val)
{
	m_show_scroll_bars=val;
	if(!m_hWnd)return;
	ShowScrollBar(SB_BOTH,m_show_scroll_bars);
	set_scroll_bars();
}


void CGisCanvas::set_scroll_bars()
{
	if(m_scroll_event_mode)return;
	SCROLLINFO si;
	si.cbSize=sizeof(SCROLLINFO);
	si.fMask=(m_show_scroll_bars?SIF_DISABLENOSCROLL:0)|SIF_PAGE|SIF_POS|SIF_RANGE;
	si.nMin=0;
	si.nMax=max_scroll-1;
	si.nPage=0;
	si.nPos=0;

	if(!m_show_scroll_bars||maximum_bound==geo::rect_inverse_infinity())
	{
		si.nPage=max_scroll;
		SetScrollInfo(SB_HORZ,&si,true);
		SetScrollInfo(SB_VERT,&si,true);
	}
	else
	{
		geo::rect mx=maximum_bound;
		world2pix(mx);
		enlarge_delta(mx);

		geo::rect rc(0,0,width,height);
		
		si.nPage=static_cast<UINT>( max_scroll*rc.width()/mx.width() );
		if(rc.x1<mx.x1)si.nPos=0;
		else si.nPos=static_cast<int>( max_scroll*(rc.x1-mx.x1)/mx.width() );
		SetScrollInfo(SB_HORZ,&si,true);
		
		si.nPage=static_cast<UINT>( max_scroll*rc.height()/mx.height() );
		if(rc.y1<mx.y1)si.nPos=0;
		else si.nPos=static_cast<int>( max_scroll*(rc.y1-mx.y1)/mx.height() );
		SetScrollInfo(SB_VERT,&si,true);
	}
}

void CGisCanvas::set_info(gis_canvas_info info)
{
	info.width=width;
	info.height=height;
	create_engines(info.engine_type,info.engines_count);
	gis_canvas::set_info(info);
}

BOOL CGisCanvas::OnToolTipNotify(UINT id, NMHDR *pNMHDR,LRESULT *pResult)
{
   TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
   if (pNMHDR->code == TTN_NEEDTEXTA)
   {
	   if(hint_visible)
		   lstrcpyn(pTTTA->szText, last_hint.c_str(), sizeof(pTTTA->szText));
	   else pTTTA->szText[0]=0;
   }
   *pResult = 0;
   return TRUE;
}

void CGisCanvas::set_maximum_extent()
{
	geo::rect rc=maximum_bound;
	double scale;
	if(angle) 
	{
		double alfa=atan2(rc.height(),rc.width())+agg::deg2rad(angle);
		double g=sqrt(rc.width()*rc.width()+rc.height()*rc.height());
		double h=g*sin(alfa);
		scale=std::max( std::max(rc.width(),h)/width,std::max(rc.height(),h)/height );
	}
	else scale=std::max( rc.width()/width,rc.height()/height );

	double d=(scroll_delta+2.0)*scale;
	rc.x1-=d;
	rc.y1-=d;
	rc.x2+=d;
	rc.y2+=d;
	set_extent(rc);
}

agg::BaseEngine& CGisCanvas::get_engine(unsigned level)
{
	if(level>=engines.size())return *engines.back();
	return *engines[level];
}

void CGisCanvas::init_cursors()
{
	add_cursor(cs_arrow,AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	add_cursor(cs_ibeam,AfxGetApp()->LoadStandardCursor(IDC_IBEAM));
	add_cursor(cs_wait,AfxGetApp()->LoadStandardCursor(IDC_WAIT));
	add_cursor(cs_cross,AfxGetApp()->LoadStandardCursor(IDC_CROSS));
	add_cursor(cs_uparrow,AfxGetApp()->LoadStandardCursor(IDC_UPARROW));
	add_cursor(cs_sizeall,AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
	add_cursor(cs_sizenwse,AfxGetApp()->LoadStandardCursor(IDC_SIZENWSE));
	add_cursor(cs_sizenesw,AfxGetApp()->LoadStandardCursor(IDC_SIZENESW));
	add_cursor(cs_sizewe,AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
	add_cursor(cs_sizens,AfxGetApp()->LoadStandardCursor(IDC_SIZENS));

	add_cursor(cs_zoomin,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_ZOOMIN)));
	add_cursor(cs_zoomout,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_ZOOMOUT)));
	add_cursor(cs_hand,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_HAND)));
	add_cursor(cs_hand_move,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_HAND_MOVE)));
	add_cursor(cs_rotate,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_ROTATE)));
	add_cursor(cs_rotate_selection,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_ROTATE_SELECTION)));
	add_cursor(cs_copy_selection,AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_GISMFC_CURSOR_COPY_SELECTION)));
}

void CGisCanvas::add_cursor(int code,HCURSOR val)
{
	if(val==0)return;
	cursors[code]=val;
}


void CGisCanvas::update_cursor_state()
{
	int ind=get_cursor();
	cursors_t::iterator it=cursors.find(ind);
	if(it!=cursors.end())SetCursor(it->second);
}

void CGisCanvas::RegisterClass()
{
	static bool is_registered = false;
	if(is_registered)return;
	
	WNDCLASS wc; 
    wc.style=CS_HREDRAW |CS_VREDRAW | CS_DBLCLKS | CS_PARENTDC | CS_GLOBALCLASS;
    wc.lpfnWndProc=(WNDPROC)iWndProc; //Creation WndProc
    wc.hInstance=::AfxGetInstanceHandle();
    wc.lpszClassName="GisCanvas";
    wc.cbClsExtra=0;
    wc.cbWndExtra=4;
    wc.hIcon=0;
    wc.hCursor=0;
    wc.hbrBackground=0;
    wc.lpszMenuName=0;
    ::AfxRegisterClass(&wc);
}

LRESULT CALLBACK CGisCanvas::iWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if(nMessage==WM_NCCREATE)
	{
		unsigned long style=GetWindowLong(hWnd,GWL_STYLE);
		WNDPROC wndProc=AfxGetAfxWndProc();
		SetWindowLong(hWnd,GWL_STYLE,style);        //Subclassing
		SetWindowLong(hWnd,GWL_WNDPROC,DWORD(wndProc));    //procedure
		CGisCanvas* p=new CGisCanvas(); //setting styles 
		p->need_delete=true;
		p->Attach(hWnd);
		SetWindowLong(hWnd, 0, (DWORD)p);    //Storring address of the  
                                                //cpp-class instance 
        return CallWindowProc(wndProc,hWnd,nMessage,wParam,lParam);
    }
	return ::DefWindowProc(hWnd, nMessage, wParam, lParam);
}

void CGisCanvas::PostNcDestroy()
{
	if(need_delete)delete this;
	CWnd::PostNcDestroy();
}

void CGisCanvasRegisterClass()
{
	CGisCanvas::RegisterClass();
}


BEGIN_MESSAGE_MAP(CGisCanvas, CWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_GETDLGCODE()
	ON_WM_SIZE()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_PRINTCLIENT,OnPrintClient)
END_MESSAGE_MAP()

// CGisCanvas message handlers


//Мышь
bool CGisMouse::is_left_down() const
{
	return (GetKeyState(VK_LBUTTON)&0x8000)!=0;
}

bool CGisMouse::is_middle_down() const
{
	return (GetKeyState(VK_MBUTTON)&0x8000)!=0;
}

bool CGisMouse::is_right_down() const
{
	return (GetKeyState(VK_RBUTTON)&0x8000)!=0;
}

int CGisMouse::getX() const
{
	POINT pt;
	if(!GetCursorPos(&pt)) return 0;
	m_parent->ScreenToClient(&pt);
	return pt.x;
}

int CGisMouse::getY() const
{
	POINT pt;
	if(!GetCursorPos(&pt)) return 0;
	m_parent->ScreenToClient(&pt);
	return pt.y;
}

shift_state_t CGisKeyboard::get_shift_state() const
{
	return m_parent->get_shift_state();
}

bool CGisKeyboard::pressed(int code) const
{
	return (GetKeyState(code)&0x8000)!=0;
}


void CGisCanvas::set_engine_type(int val)
{
	create_engines(val,engines.size());
	InvalidateAll();
}

void CGisCanvas::set_engines_count(unsigned val)
{
	create_engines(engine_type,val);
	InvalidateAll();
}

void CGisCanvas::create_engines(int eng_type,unsigned surface_count)
{
	if(surface_count>2&&is_engine_buffered(eng_type)&&
		!is_spawn_buffer_supported(eng_type)) throw e_create_engine_failed();

	engines.clear();
	if(!surface_count)return;
	
	engines.resize(surface_count);
	for(unsigned i=1;i<engines.size();i++)
	{
		engines[i]=agg::CreateEngine(eng_type,m_hWnd);
		if(window_created)engines[i]->Resize(width,height,m_hWnd);
	}
	engines[0]=agg::CreateEngine(get_foreground_engine(eng_type),m_hWnd);
	if(window_created)engines[0]->Resize(width,height,m_hWnd);

	engine_type=eng_type;
	engines_count=surface_count;
}

void CGisCanvas::hint_init()
{
	hint_delay=def_cursor_delay;
	hintX=-1;
	hintY=-1;
	hint_visible=false;
}

void CGisCanvas::hint_create()
{
	m_tooltip.Create(this);
	CRect tool_rect;
	GetClientRect(&tool_rect);
	m_tooltip.SetMaxTipWidth(300);
	m_tooltip.AddTool(this,LPSTR_TEXTCALLBACK,&tool_rect,1);
	TOOLINFO ti;
	ti.cbSize=sizeof(TOOLINFO);
	ti.hwnd=m_hWnd;
	ti.lpszText=LPSTR_TEXTCALLBACK;
	ti.uId=1;
	ti.rect=tool_rect;
	ti.uFlags=TTF_TRACK|TTF_ABSOLUTE;
	::SendMessage(m_tooltip.m_hWnd,TTM_ADDTOOL,(WPARAM)1,(LPARAM)&ti);
	::SendMessage(m_tooltip.m_hWnd,TTM_TRACKACTIVATE,(WPARAM)1,(LPARAM)&ti);
}


void CGisCanvas::hint_mouse_move_process()
{
	if(!hint_can_be_displayed())
	{
		KillTimer(hint_timer_id);
		hintX=-1;
		hint_show(false);
		return;
	}
	if(hintX==m_mouse.getX()&&hintY==m_mouse.getY())return;
	KillTimer(hint_timer_id);
	hint_show(false);
	hintX=m_mouse.getX();
	hintY=m_mouse.getY();

	::SendMessage(m_tooltip.m_hWnd,TTM_TRACKPOSITION,0,(LPARAM)MAKELPARAM(hintX,hintY));

	SetTimer(hint_timer_id,hint_delay,0);
}

void CGisCanvas::hint_timer_process()
{
	KillTimer(hint_timer_id);
	if(!hint_can_be_displayed())return;
	int nx=m_mouse.getX();
	int ny=m_mouse.getY();
	if(hintX!=nx||hintY!=ny)return;
	std::string hint;
	OnHint(*this,hintX,hintY,hint);
	bool ch=hint!=last_hint;
	last_hint=hint;
	if(last_hint.empty())hint_show(false);
	else if(ch||!m_tooltip.IsWindowVisible())hint_show(true);
}

void CGisCanvas::hint_show(bool val)
{
	hint_visible=val;
	m_tooltip.Update();
}


bool CGisCanvas::hint_can_be_displayed()
{
	if(!show_hints||OnHint.empty())return false;
	if(!IsWindowVisible())return false;
	if(m_mouse.is_left_down()||
		m_mouse.is_middle_down()||
		m_mouse.is_right_down())return false;
	POINT p;
	if(!GetCursorPos(&p)) return false;
	RECT rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	if(p.x<rc.left||p.x>rc.right||p.y<rc.top||p.y>rc.bottom)
		return false;
	return true;
}


}//namespace GisCommon

void GisCommon::CGisCanvas::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseDoubleClick(m_mouse,point.x,point.y,bLeft,st);
		CWnd::OnLButtonDblClk(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnLButtonDown(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		SetCapture();
		m_mouse.OnMouseDown(m_mouse,point.x,point.y,bLeft,st);
		CWnd::OnLButtonDown(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnLButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		ReleaseCapture();
		m_mouse.OnMouseUp(m_mouse,point.x,point.y,bLeft,st);
		CWnd::OnLButtonUp(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseDoubleClick(m_mouse,point.x,point.y,bMiddle,st);
		CWnd::OnMButtonDblClk(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnMButtonDown(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseDown(m_mouse,point.x,point.y,bMiddle,st);
		CWnd::OnMButtonDown(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnMButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseUp(m_mouse,point.x,point.y,bMiddle,st);
		CWnd::OnMButtonUp(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseDoubleClick(m_mouse,point.x,point.y,bRight,st);
		CWnd::OnRButtonDblClk(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnRButtonDown(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseDown(m_mouse,point.x,point.y,bRight,st);
		CWnd::OnRButtonDown(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnRButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseUp(m_mouse,point.x,point.y,bRight,st);
		CWnd::OnRButtonUp(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnMouseMove(UINT nFlags, CPoint point)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		shift_state_t st=flags2shift_state(nFlags);
		m_mouse.OnMouseMove(m_mouse,point.x,point.y,st);
		CWnd::OnMouseMove(nFlags, point);
		hint_mouse_move_process();
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnPaint()
{
	try
	{
		RECT rc;
		//согласно описанию в MFC
		if(!GetUpdateRect(&rc))return;
		CPaintDC dc(this); // device context for painting

		if(!window_created||engines.empty())return;

		unsigned invalidate_level=std::min(pended_invalidate_level,engines.size()-1);
		pended_invalidate_level=0;
		draw_start_level=invalidate_level;
		do_draw(dc,invalidate_level);
	}
	catch(std::exception& e)
	{
		ObjectProgress::object_generator_impl<CGisCanvas> lg(*this,true);
		lg<<"CGisCanvas::OnPaint() std::exception: "<<typeid(e).name()<<" : "<<e.what();
	}
	catch(...)
	{
		ObjectProgress::object_generator_impl<CGisCanvas> lg(*this,true);
		lg<<"CGisCanvas::OnPaint() unknown exception";
	}
}

void GisCommon::CGisCanvas::do_draw(CDC& dc,unsigned invalidate_level)
{
		if(!window_created||engines.empty())return;
		int max_int=std::numeric_limits<int>::max();

		double dx_offset=world_x/scale;
		if(fabs(dx_offset)>std::numeric_limits<int>::max())
			dx_offset=dx_offset-floor(dx_offset/max_int)*max_int;
		unsigned x_offset=static_cast<int>(dx_offset);

		double dy_offset=world_y/scale;
		if(fabs(dy_offset)>std::numeric_limits<int>::max())
			dy_offset=dy_offset-floor(dy_offset/max_int)*max_int;
		unsigned y_offset=std::numeric_limits<unsigned>::max()-static_cast<int>(dy_offset);

		ObjectProgress::object_generator_impl<CGisCanvas> lg(*this,true);
		ObjectProgress::progress_discret prg(invalidate_level+1,lg);


		if(is_engine_buffered(engine_type))
		{
			for(;invalidate_level>0;--invalidate_level,prg.next_pos())
			{
				BaseEngine& engine=*engines[invalidate_level];
				agg::auto_draw ad(engine);
				engine.SetBrushOffset(x_offset,y_offset);

				if(invalidate_level==engines.size()-1)
					engine.FillBackground(background_color);
				else if(is_spawn_buffer_supported(engine_type))
				{
					blt_param_t blt(width,height);
					OnBlitingParams(*this,blt.offset_x,blt.offset_y,blt.scale,blt.angle,invalidate_level);
					blt.calculate_blt();
						
					if(!blt.no_transform())engine.FillBackground(background_color);

					ad.reset();

					if(blt.is_blit())
						engine.GetBuffer()->BitBlt(
							engines[invalidate_level+1]->GetBuffer(),
							blt.dst_x,blt.dst_y,blt.dst_width,blt.dst_height,
							blt.src_x,blt.src_y,blt.src_width,blt.src_height);
					ad.set(engine);
				}
				OnDraw(*this,engine,invalidate_level);
			}

			if(engines.size()>=2)
			{
				BaseEngine& engine=*engines[1];
				
				blt_param_t blt(width,height);
				OnBlitingParams(*this,blt.offset_x,blt.offset_y,blt.scale,blt.angle,0);
				blt.calculate_blt();
				if(blt.no_transform())engine.Flush(dc.m_hDC);
				else
				{
					engines[0]->FillBackground(background_color);
					engine.Flush(dc.m_hDC,
						blt.dst_x,blt.dst_y,blt.dst_width,blt.dst_height,
						blt.src_x,blt.src_y,blt.src_width,blt.src_height);
				}
			}
		}
		//Не буфферизированный background
		else
		{
			//Не буфферизированный вывод требует отрисовки всех слоёв
			invalidate_level=engines.size()-1;

			for(;invalidate_level>0;--invalidate_level,prg.next_pos())
			{
				BaseEngine& engine=*engines[invalidate_level];
				engine.SetBrushOffset(x_offset,y_offset);
				agg::auto_draw ad(engine);

				if(invalidate_level==engines.size()-1)
					engine.FillBackground(background_color);
				OnDraw(*this,engine,invalidate_level);
			}
		}

		//рисуем foreground
		BaseEngine& foreground_engine=*engines[0];
		agg::auto_draw ad(foreground_engine);
		foreground_engine.SetBrushOffset(x_offset,y_offset);
		OnDraw(*this,foreground_engine,0);
		ad.reset();
		OnAfterDraw(*this);
}



void GisCommon::CGisCanvas::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	try
	{
		switch(nSBCode)
		{
		case SB_LEFT:
			SetScrollPos(SB_HORZ,GetScrollPos(SB_HORZ)-max_scroll/4);
			break;
		case SB_RIGHT:
			SetScrollPos(SB_HORZ,GetScrollPos(SB_HORZ)+max_scroll/4);
			break;
		case SB_LINELEFT:
			SetScrollPos(SB_HORZ,GetScrollPos(SB_HORZ)-max_scroll/256);
			break;
		case SB_LINERIGHT:
			SetScrollPos(SB_HORZ,GetScrollPos(SB_HORZ)+max_scroll/256);
			break;
		case SB_PAGELEFT:
			SetScrollPos(SB_HORZ,GetScrollPos(SB_HORZ)-max_scroll/32);
			break;
		case SB_PAGERIGHT:
			SetScrollPos(SB_HORZ,GetScrollPos(SB_HORZ)+max_scroll/32);
			break;
		case SB_THUMBPOSITION:
			SetScrollPos(SB_HORZ,nPos);
			break;
		case SB_ENDSCROLL:
			int pos=GetScrollPos(SB_HORZ);
			geo::rect mx=maximum_bound;
			world2pix(mx);
			enlarge_delta(mx);
			geo::point pt(mx.x1+pos*mx.width()/max_scroll+width/2.0,height/2.0);
			m_scroll_event_mode=true;
			try
			{
				recenter_pixel(pt.x,pt.y);
			}
			catch(...)
			{
				m_scroll_event_mode=false;
				throw;
			}
			m_scroll_event_mode=false;
			break;
		}
		CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	try
	{
		switch(nSBCode)
		{
		case SB_TOP:
			SetScrollPos(SB_VERT,GetScrollPos(SB_VERT)-max_scroll/4);
			break;
		case SB_BOTTOM:
			SetScrollPos(SB_VERT,GetScrollPos(SB_VERT)+max_scroll/4);
			break;
		case SB_LINEUP:
			SetScrollPos(SB_VERT,GetScrollPos(SB_VERT)-max_scroll/256);
			break;
		case SB_LINEDOWN:
			SetScrollPos(SB_VERT,GetScrollPos(SB_VERT)+max_scroll/256);
			break;
		case SB_PAGEUP:
			SetScrollPos(SB_VERT,GetScrollPos(SB_VERT)-max_scroll/32);
			break;
		case SB_PAGEDOWN:
			SetScrollPos(SB_VERT,GetScrollPos(SB_VERT)+max_scroll/32);
			break;
		case SB_THUMBPOSITION:
			SetScrollPos(SB_VERT,nPos);
			break;
		case SB_ENDSCROLL:
			int pos=GetScrollPos(SB_VERT);
			geo::rect mx=maximum_bound;
			world2pix(mx);
			enlarge_delta(mx);
			geo::point pt(width/2.0,mx.y1+pos*mx.height()/max_scroll+height/2.0);
			m_scroll_event_mode=true;
			try
			{
				recenter_pixel(pt.x,pt.y);
			}
			catch(...)
			{
				m_scroll_event_mode=false;
				throw;
			}
			m_scroll_event_mode=false;
			break;
		}
		CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
	}
	GISMFC_EXCEPTION_CATCH;
}


void GisCommon::CGisCanvas::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		m_keyboard.OnChar(m_keyboard,nChar,get_shift_state());
		CWnd::OnChar(nChar, nRepCnt, nFlags);
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		m_keyboard.OnKeyDown(m_keyboard,nChar,get_shift_state());

		if(nChar==VK_ESCAPE)m_keyboard.OnEscape(m_keyboard,nChar,get_shift_state());
		else if(nChar==VK_RETURN)m_keyboard.OnEnter(m_keyboard,nChar,get_shift_state());
		else if(nChar==VK_SPACE)m_keyboard.OnSpace(m_keyboard,nChar,get_shift_state());
		
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
	GISMFC_EXCEPTION_CATCH;
}

void GisCommon::CGisCanvas::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	try
	{
		// TODO: Add your message handler code here and/or call default
		m_keyboard.OnKeyUp(m_keyboard,nChar,get_shift_state());
		CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	GISMFC_EXCEPTION_CATCH;
}

UINT GisCommon::CGisCanvas::OnGetDlgCode()
{
	// TODO: Add your message handler code here and/or call default
	return DLGC_WANTALLKEYS;
}

BOOL GisCommon::CGisCanvas::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	int ind=get_cursor();
	if(nHitTest==HTCLIENT&&ind>=0&&ind<(int)cursors.size())
	{
		SetCursor(cursors[ind]);
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL GisCommon::CGisCanvas::PreTranslateMessage(MSG* pMsg)
{
    m_tooltip.RelayEvent(pMsg);
	return CWnd::PreTranslateMessage(pMsg);
}

void GisCommon::CGisCanvas::OnTimer(UINT nIDEvent)
{
	try
	{
		if(nIDEvent==hint_timer_id)hint_timer_process();
	}
	GISMFC_EXCEPTION_CATCH;
	CWnd::OnTimer(nIDEvent);
}

void GisCommon::CGisCanvas::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	on_context_menu(*this,point.x,point.y);
}

LRESULT GisCommon::CGisCanvas::OnPrintClient(WPARAM wParam, LPARAM lParam)
{
	try
	{
		if(!(lParam&PRF_CLIENT))return 0;
		CDC dc;
		dc.Attach(reinterpret_cast<HDC>(wParam));
		do_draw(dc,0);
		dc.Detach();
	}
	catch(std::exception& e)
	{
		ObjectProgress::object_generator_impl<CGisCanvas> lg(*this,true);
		lg<<"CGisCanvas::OnPrintClient() std::exception: "<<typeid(e).name()<<" : "<<e.what();
	}
	catch(...)
	{
		ObjectProgress::object_generator_impl<CGisCanvas> lg(*this,true);
		lg<<"CGisCanvas::OnPrintClient() unknown exception";
	}
	return 0;
}
