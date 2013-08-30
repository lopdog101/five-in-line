#include "stdafx.h"
#include "resource.h"
#include "InterractivePlayers.h"

namespace Gomoku
{

void mfcPlayer::init(game_t& _gm,Step _cl)
{
	iplayer_t::init(_gm,_cl);
	fd=&dynamic_cast<CMfcField&>(*game().fieldp);

    if(!inited)
	{
		if(color()==st_krestik)bmp.LoadBitmap(IDB_KRESTIK_HI);
		else bmp.LoadBitmap(IDB_NOLIK_HI);
		inited=true;
	}
}


void mfcPlayer::fieldLMouseUp(point pos)
{
	if(game().field().at(pos)!=st_empty)return;
    reset_handlers();
    game().OnNextStep(*this,pos);
}

void mfcPlayer::reset_handlers()
{
    hld_mouse_down.disconnect();
	hld_mouse_up.disconnect();
	hld_mouse_move.disconnect();
	hld_after_draw.disconnect();
}


void mfcPlayer::afterDraw(CDC& dc)
{
	CPoint pos=fd->mouse_pos();
	point pt=fd->pix2world(pos);
	if(game().field().at(pt)!=st_empty)return;
	pos=fd->world2pix(pt);

	CDC bmp_dc;
	bmp_dc.CreateCompatibleDC(&dc);
	bmp_dc.SelectObject(bmp);
	dc.BitBlt(pos.x,pos.y,CMfcField::box_size,CMfcField::box_size,&bmp_dc,0,0,SRCCOPY);
}

void mfcPlayer::fieldMouseMove(point pos)
{
	fd->Invalidate();
}

void mfcPlayer::fieldLMouseDown(point pos)
{
	hld_mouse_up=fd->OnLMouseUp.connect(boost::bind(&mfcPlayer::fieldLMouseUp,this,_1) );
}

void mfcPlayer::delegate_step()
{
	hld_mouse_down=fd->OnLMouseDown.connect(boost::bind(&mfcPlayer::fieldLMouseDown,this,_1) );
	hld_mouse_move=fd->on_mouse_move.connect(boost::bind(&mfcPlayer::fieldMouseMove,this,_1) );
	hld_after_draw=fd->on_after_paint.connect(boost::bind(&mfcPlayer::afterDraw,this,_1) );
}

void mfcPlayer::request_cancel(bool val)
{
    if(!val)return;

    reset_handlers();
}

bool mfcPlayer::is_thinking() const
{
    return hld_mouse_down.connected() || hld_mouse_up.connected();
}

// ThreadPlayer
void ThreadPlayer::clear()
{
    hld_execute.disconnect();
    hld_complete.disconnect();
    hld_errors.disconnect();
    hld_mirror_next_step.disconnect();
    if(pl)pl->request_cancel(true);
	processor.stop();
    if(pl)pl->request_cancel(false);
}

void ThreadPlayer::init(game_t& _gm,Step _cl)
{
	clear();

	iplayer_t::init(_gm,_cl);
	mirror_gm.field()=_gm.field();

	null_player=player_ptr(new NullPlayer);
	
	if(_cl==st_krestik)
	{
		mirror_gm.set_krestik(pl);
		mirror_gm.set_nolik(null_player);
	}
	else
	{
		mirror_gm.set_krestik(null_player);
		mirror_gm.set_nolik(pl);
	}

	pl->init(mirror_gm,_cl);

	hld_execute=processor.OnExecute.connect(boost::bind(&ThreadPlayer::MirrorExecute,this) );
	hld_complete=processor.OnComplete.connect(boost::bind(&ThreadPlayer::TaskComplete,this) );
	hld_errors=processor.OnErrors.connect(boost::bind(&ThreadPlayer::TaskErrors,this,_1) );

    hld_mirror_next_step=mirror_gm.OnNextStep.connect(boost::bind(&ThreadPlayer::mirrorNextStep,this,_1,_2) );

	processor.start();
}

void ThreadPlayer::delegate_step()
{
    pl->request_cancel(true);
	processor.cancel_job();
    pl->request_cancel(false);
    
	mirror_gm.field()=game().field();
    answer_complete = false;
	processor.start_job();
}

void ThreadPlayer::request_cancel(bool val)
{
    if(!val)return;

    pl->request_cancel(true);
	processor.cancel_job();
    pl->request_cancel(false);
}

bool ThreadPlayer::is_thinking() const
{
    return processor.is_job_in_progress();
}

void ThreadPlayer::MirrorExecute()
{
    try
    {
        pl->delegate_step();
        answer_complete = true;
    }
    catch(e_cancel&)
    {
    }
}

void ThreadPlayer::mirrorNextStep(const iplayer_t& pl,const point& pt)
{
    answer_point=pt;
}


void ThreadPlayer::TaskComplete()
{
    if(answer_complete)
    {
        game().OnNextStep(*this,answer_point);
    }
}

void ThreadPlayer::TaskErrors(const CThreadProcessor::errors_t& vals)
{
	AfxMessageBox(vals.front().message.c_str());
}

}//namespace