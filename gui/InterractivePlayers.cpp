#include "stdafx.h"
#include "resource.h"
#include "InterractivePlayers.h"


void mfcPlayer::init(Gomoku::game_t& _gm,Gomoku::Step _cl)
{
	Gomoku::iplayer_t::init(_gm,_cl);
	fd=&dynamic_cast<CMfcField&>(gm->field());
	if(!inited)
	{
		if(color()==Gomoku::st_krestik)bmp.LoadBitmap(IDB_KRESTIK_HI);
		else bmp.LoadBitmap(IDB_NOLIK_HI);
		inited=true;
	}
}


void mfcPlayer::fieldLMouseUp(Gomoku::point pos)
{
	if(gm->field().at(pos)!=Gomoku::st_empty)return;
    hld_mouse_down.disconnect();
	hld_mouse_up.disconnect();
	hld_mouse_move.disconnect();
	hld_after_draw.disconnect();
	gm->make_step(*this,pos);
}

void mfcPlayer::afterDraw(CDC& dc)
{
	CPoint pos=fd->mouse_pos();
	Gomoku::point pt=fd->pix2world(pos);
	if(gm->field().at(pt)!=Gomoku::st_empty)return;
	pos=fd->world2pix(pt);

	CDC bmp_dc;
	bmp_dc.CreateCompatibleDC(&dc);
	bmp_dc.SelectObject(bmp);
	dc.BitBlt(pos.x,pos.y,CMfcField::box_size,CMfcField::box_size,&bmp_dc,0,0,SRCCOPY);
}

void mfcPlayer::fieldMouseMove(Gomoku::point pos)
{
	fd->Invalidate();
}

void mfcPlayer::fieldLMouseDown(Gomoku::point pos)
{
	hld_mouse_up=fd->OnLMouseUp.connect(boost::bind(&mfcPlayer::fieldLMouseUp,this,_1) );
}

void mfcPlayer::delegate_step()
{
	hld_mouse_down=fd->OnLMouseDown.connect(boost::bind(&mfcPlayer::fieldLMouseDown,this,_1) );
	hld_mouse_move=fd->on_mouse_move.connect(boost::bind(&mfcPlayer::fieldMouseMove,this,_1) );
	hld_after_draw=fd->on_after_paint.connect(boost::bind(&mfcPlayer::afterDraw,this,_1) );
}

// ThreadPlayer
void ThreadPlayer::clear()
{
    hld_execute.disconnect();
    hld_complete.disconnect();
    hld_errors.disconnect();
	if(pl)pl->cancel();
	processor.stop();
}



void ThreadPlayer::init(Gomoku::game_t& _gm,Gomoku::Step _cl)
{
	clear();

	Gomoku::iplayer_t::init(_gm,_cl);
	mirror_gm.field()=_gm.field();

	null_player=Gomoku::player_ptr(new NullPlayer);
	
	if(_cl==Gomoku::st_krestik)
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

	processor.start();
}

void ThreadPlayer::delegate_step()
{
	pl->cancel();
	processor.cancel_job();

	pl->begin_game();


	mirror_gm.field()=gm->field();
	processor.start_job();
}

void ThreadPlayer::MirrorExecute()
{
	pl->delegate_step();
}

void ThreadPlayer::TaskComplete()
{
	gm->make_step(*this,mirror_gm.field().back());
}

void ThreadPlayer::TaskErrors(const CThreadProcessor::errors_t& vals)
{
	AfxMessageBox(vals.front().message.c_str());
}

